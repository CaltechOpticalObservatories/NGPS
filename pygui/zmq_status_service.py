import zmq
import os
import logging
import json
from PyQt5.QtCore import pyqtSignal, QObject, QThread
from typing import Dict, Any

class ZmqStatusService(QObject):
    # Legacy/debug signal. Keep this separate from the operator-facing log.
    new_message_signal = pyqtSignal(str)

    # Operator-facing message-log signal. This is emitted for every valid
    # subscribed topic broadcast received from the ZMQ broker.
    topic_broadcast_signal = pyqtSignal(str)

    # Signal to send lamp states as a dictionary {lamp_name: bool}
    lamp_states_signal = pyqtSignal(dict)
    
    # Signal to send modulator states as a dictionary {modulator_name: bool}
    modulator_states_signal = pyqtSignal(dict)

    airmass_signal = pyqtSignal(float) 
    
    slit_info_signal = pyqtSignal(float, float)
    
    system_status_signal = pyqtSignal(str)

    # Emitted when sequencerd says it is waiting for the USER continue signal.
    # This replaces the old UDP StatusService detection path for:
    # waiting for USER to send "continue" signal
    user_can_expose_signal = pyqtSignal(bool)

    # seqgui-style daemon status signals
    daemonstate_signal = pyqtSignal(dict)     # seq_daemonstate: daemon-key -> bool
    waitstate_signal = pyqtSignal(dict)       # seq_waitstate: wait-key -> bool; daemon/status bar only
    sequencerd_alive_signal = pyqtSignal()    # emitted when seq_seqstate arrives

    def __init__(
        self,
        parent,
        broker_publish_endpoint="tcp://127.0.0.1:5556",
        broker_snapshot_endpoint="tcp://127.0.0.1:5555",
        emit_debug_messages=False,
        emit_topic_broadcasts=True,
    ):
        super().__init__()
        self.parent = parent  # Reference to the parent window or main UI
        self.broker_publish_endpoint = broker_publish_endpoint
        self.broker_snapshot_endpoint = broker_snapshot_endpoint

        # Debug/raw-message output flag for legacy/debug consumers.
        # This is intentionally not the operator-facing broadcast log.
        self.emit_debug_messages = emit_debug_messages

        # Operator-facing broadcast log flag.
        # True = emit received subscribed topic/payload messages through
        # topic_broadcast_signal. This is the only signal the GUI message log
        # should connect to.
        self.emit_topic_broadcasts = bool(emit_topic_broadcasts)

        self.context = zmq.Context()
        self.socket = None
        self.pub_socket = None
        self.is_connected = False
        self.subscribed_topics = set()  # Set of subscribed topics
        # Instrument Status is intentionally driven only by seq_seqstate.
        # seq_waitstate feeds the daemon/status chip row and operator controls,
        # but it must not override the Instrument Status panel.
        self._last_seq_lifecycle_status = "stopped"
        self._user_continue_ready_emitted = False

        # Set up logging
        self.setup_logging()
        self.logger.info("StatusService initialized.")

    def set_debug_messages(self, enabled: bool):
        """
        Enable or disable legacy raw/debug ZMQ messages.

        This does not control the operator-facing topic broadcast log.
        """
        self.emit_debug_messages = bool(enabled)

    def set_topic_broadcast_messages(self, enabled: bool):
        """
        Enable or disable the operator-facing topic broadcast log signal.
        """
        self.emit_topic_broadcasts = bool(enabled)

    def _emit_debug_message(self, message: str):
        """
        Emit raw/debug messages only when debug output is enabled.
        """
        if self.emit_debug_messages:
            self.new_message_signal.emit(message)

    def _emit_topic_broadcast_message(self, topic: str, payload: str):
        """
        Emit the exact subscribed topic broadcast to the GUI message log.

        This is intentionally independent from JSON parsing and debug output,
        so malformed/non-JSON broadcasts can still be shown to the observer.
        """
        if self.emit_topic_broadcasts:
            self.topic_broadcast_signal.emit(f"Topic: {topic}, Payload: {payload}")

    @staticmethod
    def _contains_user_continue_message(value) -> bool:
        """
        Return True if a raw or decoded ZMQ payload contains the sequencer
        message that tells the GUI the operator can press Expose/Continue.
        """
        needle = 'waiting for USER to send "continue" signal'

        if value is None:
            return False

        if isinstance(value, dict):
            return any(ZmqStatusService._contains_user_continue_message(v) for v in value.values())

        if isinstance(value, (list, tuple, set)):
            return any(ZmqStatusService._contains_user_continue_message(v) for v in value)

        return needle in str(value)

    def _emit_user_can_expose_once(self):
        """
        Emit user_can_expose_signal once per USER wait-state/message.
        Repeated ZMQ publications should not repeatedly restyle/pop the GUI.
        """
        if not self._user_continue_ready_emitted:
            self.logger.info('Sequencer is waiting for USER continue; enabling Expose/Offset controls.')
            self.user_can_expose_signal.emit(True)
            self._user_continue_ready_emitted = True

    def _clear_user_can_expose_latch(self):
        """
        Allow a future USER wait-state/message to emit again after the
        sequencer leaves the USER wait state.
        """
        self._user_continue_ready_emitted = False

    def setup_logging(self):
        """ Set up logging for the status service in a 'logs' folder. """
        
        # Ensure the 'logs' directory exists
        log_dir = 'logs'
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)  # Create the logs directory if it doesn't exist

        # Set up logging to a file inside the 'logs' folder
        log_file = os.path.join(log_dir, 'zmq_status_service.log')
        
        logging.basicConfig(
            level=logging.INFO, 
            format='%(asctime)s - %(levelname)s - %(message)s',
            filename=log_file,  # Log file inside 'logs' folder
            filemode='a'  # 'a' to append, 'w' to overwrite
        )
        
        self.logger = logging.getLogger(__name__)

    def connect(self):
        """ Connect to the broker using the SUB socket (not XSUB). """
        try:
            self.logger.info(f"Connecting to broker at {self.broker_publish_endpoint}...")
            # Create the SUB socket type to receive messages
            self.socket = self.context.socket(zmq.SUB)
            self.socket.connect(self.broker_publish_endpoint)  # Connect to the broker (publisher's address and port)

            # Optional PUB socket used only to request a status snapshot, matching seqgui.
            # If nobody is listening on this endpoint, send_multipart simply has no effect.
            self.pub_socket = self.context.socket(zmq.PUB)
            self.pub_socket.connect(self.broker_snapshot_endpoint)

            self.is_connected = True
            self.logger.info(f"Connected to broker at {self.broker_publish_endpoint}")
        except Exception as e:
            self.logger.error(f"Failed to connect to broker: {e}")
            self.is_connected = False
            raise e

    def subscribe_to_topic(self, topic):
        """ Subscribe to a specific topic. """
        if not self.is_connected:
            self.logger.warning("Not connected to broker. Call 'connect()' first.")
            return
        
        if topic not in self.subscribed_topics:
            self.socket.setsockopt_string(zmq.SUBSCRIBE, topic)  # Subscribe to the given topic
            self.subscribed_topics.add(topic)
            self.logger.info(f"Subscribed to topic: {topic}")
        else:
            self.logger.info(f"Already subscribed to topic: {topic}")

    def unsubscribe_from_topic(self, topic):
        """ Unsubscribe from a specific topic. """
        if not self.is_connected:
            self.logger.warning("Not connected to broker. Call 'connect()' first.")
            return
        
        if topic in self.subscribed_topics:
            self.socket.setsockopt_string(zmq.UNSUBSCRIBE, topic)  # Unsubscribe from the given topic
            self.subscribed_topics.remove(topic)
            self.logger.info(f"Unsubscribed from topic: {topic}")
        else:
            self.logger.info(f"Not subscribed to topic: {topic}")

    def subscribe_to_all(self):
        """ Subscribe to all topics. """
        if not self.is_connected:
            self.logger.warning("Not connected to broker. Call 'connect()' first.")
            return
        
        self.socket.setsockopt_string(zmq.SUBSCRIBE, "")  # Subscribe to all messages
        self.subscribed_topics.clear()  # Clear the current subscriptions
        self.logger.info("Subscribed to all topics.")


    @staticmethod
    def _bool_payload(data):
        """Return only boolean fields from a decoded JSON status payload."""
        if not isinstance(data, dict):
            return {}
        return {str(k): bool(v) for k, v in data.items() if isinstance(v, bool)}

    def _request_snapshot(self):
        """Ask daemons/sequencer to re-publish current telemetry, like seqgui."""
        if self.pub_socket is None:
            return

        payload = json.dumps({
            "sequencerd": True,
            "acamd": True,
            "calibd": True,
            "camerad": True,
            "flexured": True,
            "focusd": True,
            "powerd": True,
            "slicecamd": True,
            "slitd": True,
            "tcsd": True,
            "thermald": True,
        })

        try:
            self.pub_socket.send_multipart([b"_snapshot", payload.encode("utf-8")])
        except zmq.ZMQError as e:
            self.logger.warning(f"Snapshot request failed: {e}")

    def listen(self):
        """Listen for incoming messages from the broker."""
        if not self.is_connected:
            self.logger.warning("Not connected to broker. Call 'connect()' first.")
            return

        try:
            self.logger.info("Starting to listen for messages from the broker...")

            # Give subscriptions a moment to register, then request current state.
            QThread.msleep(200)
            self._request_snapshot()

            while True:
                message = self.socket.recv_multipart()

                if len(message) != 2:
                    self.logger.warning("Received malformed message (not two parts).")
                    continue

                topic = message[0].decode("utf-8")
                payload = message[1].decode("utf-8")

                # Always log to file, even when GUI debug messages are disabled.
                self.logger.info(f"Received message: Topic = {topic}, Payload = {payload}")

                # The GUI message log should show subscribed ZMQ topic broadcasts,
                # not local GUI/status chatter. Emit this before JSON parsing so
                # plain-text broadcasts and malformed JSON still appear.
                self._emit_topic_broadcast_message(topic, payload)

                # Some sequencerd messages are plain text status messages rather
                # than JSON objects. Detect the USER continue message before
                # json.loads so raw text payloads are handled too.
                if topic == "sequencerd" and self._contains_user_continue_message(payload):
                    self._emit_user_can_expose_once()

                try:
                    data = json.loads(payload)

                    # Also support JSON payloads that contain the same text in a
                    # field such as {"message": "..."} or {"status": "..."}.
                    if topic == "sequencerd" and self._contains_user_continue_message(data):
                        self._emit_user_can_expose_once()

                    if topic == "acamd":
                        self._emit_debug_message(f"Topic: {topic}, Payload: {payload}")

                    elif topic == "seq_seqstate":
                        # Any seq_seqstate message proves sequencerd is alive.
                        # It is also the only ZMQ source allowed to drive the
                        # Instrument System Status panel. Do not mix in
                        # seq_waitstate here; wait-state belongs to the daemon
                        # chip/status bar only.
                        self.sequencerd_alive_signal.emit()
                        self._last_seq_lifecycle_status = self._status_from_seq_seqstate(data)
                        self.system_status_signal.emit(self._last_seq_lifecycle_status)

                    elif topic == "seq_waitstate":
                        # seqgui-style busy/blink source for the daemon/status
                        # chip bar only. This must not update Instrument Status.
                        wait_flags = self._bool_payload(data)
                        self.waitstate_signal.emit(wait_flags)

                        # Keep USER wait-state for enabling Continue/Expose
                        # controls, but do not display USER in Instrument Status.
                        if wait_flags.get("USER", False):
                            self._emit_user_can_expose_once()
                        else:
                            self._clear_user_can_expose_latch()

                    elif topic == "seq_daemonstate":
                        # seqgui-style ready/not-ready source.
                        self.daemonstate_signal.emit(self._bool_payload(data))

                    if topic == "slitd":
                        slit_width = data.get("SLITW", None)
                        slit_offset = data.get("SLITO", None)

                        if slit_width is not None and slit_offset is not None:
                            self.slit_info_signal.emit(slit_width, slit_offset)

                    if topic == "calibd":
                        self._emit_debug_message(f"Topic: {topic}, Payload: {payload}")
                        self.update_modulator_states(data)

                    if topic == "powerd":
                        self._emit_debug_message(f"Topic: {topic}, Payload: {payload}")
                        self.update_lamp_states(data)

                    if topic == "tcsd":
                        self.update_tcs_info(data)

                except json.JSONDecodeError as e:
                    self.logger.error(f"Error parsing JSON payload from topic '{topic}': {e}")

        except Exception as e:
            self.logger.error(f"Error while listening for messages: {e}")

        finally:
            self.disconnect()

    def disconnect(self):
        """ Disconnect from the broker and close the socket. """
        if self.socket:
            self.socket.close()
            self.socket = None

        if self.pub_socket:
            self.pub_socket.close()
            self.pub_socket = None

        self.is_connected = False
        self.logger.info("Disconnected from broker.")

    def update_lamp_states(self, data):
        """ Emit signal with the lamp states """
        if not isinstance(data, dict):
            self.logger.error("Invalid data format received for lamp states.")
            return
        
        lamp_states = {}
        # List of lamps we are interested in
        lamp_keys = ["LAMPBLUC", "LAMPFEAR", "LAMPREDC", "LAMPTHAR"]

        for lamp_key in lamp_keys:
            # Get the lamp state from the data (default to False if not found)
            lamp_states[lamp_key] = data.get(lamp_key, False)

        # Emit the signal for lamp states
        self.lamp_states_signal.emit(lamp_states)
        print(f"Emitting modulator states: {lamp_states}")

    def update_modulator_states(self, modulator_data):
        """ Emit signal with the modulator states """
        if not isinstance(modulator_data, dict):
            self.logger.error("Invalid data format received for modulator states.")
            return
        
        modulator_states = {}
        # List of modulators we are interested in
        modulator_keys = ["MODTHAR", "MODFEAR", "MODRDCON", "MODBLCON"]

        for modulator_key in modulator_keys:
            # Get the status from the modulator data
            modulator_status = modulator_data.get(modulator_key, "err")  # Default to "err" if not found

            # If "err" is in the status, consider it as the hardware not ready
            if "err" in modulator_status:
                modulator_states[modulator_key] = False  # Hardware is not ready, so set to False (off)
            else:
                # Otherwise, treat the modulator as "on" or "off"
                # If it contains the term "on", we consider it as True (on)
                modulator_states[modulator_key] = modulator_status.startswith("on")

        # Emit the signal for modulator states
        self.modulator_states_signal.emit(modulator_states)
        print(f"Emitting modulator states: {modulator_states}")

    def update_tcs_info(self, data):
        """ Handle and process the 'tcsd' message and payload. """
        # Extract relevant data or handle it as needed
        airmass = data.get('AIRMASS', None)
        alt = data.get('ALT', None)
        az = data.get('AZ', None)
        dec = data.get('DEC', None)
        domeaz = data.get('DOMEAZ', None)
        domeshut = data.get('DOMESHUT', None)
        ra = data.get('RA', None)
        raoffset = data.get('RAOFFSET', None)
        zenangle = data.get('ZENANGLE', None)

        # Here you can process or log the data, for example:
        # print(f"Processing TCS info: AIRMASS = {airmass}, ALT = {alt}, AZ = {az}, RAOFFSET = {raoffset}, etc.")

        # Emit the TCS info to the UI thread (TODO)
        # self.new_message_signal.emit(f"TCS Info: {data}")

        # Emit the AIRMASS value as a dedicated signal if available
        if airmass is not None:
            self.airmass_signal.emit(airmass)
        else:
            self.logger.warning("AIRMASS data is not available.")

    def _status_from_seq_seqstate(self, data: Dict[str, Any]) -> str:
        """
        Parse the overall sequencer lifecycle state.
        """
        if not isinstance(data, dict):
            return "stopped"

        seqstate = str(data.get("seqstate", "")).strip().upper()

        state_map = {
            "NOTREADY": "not_ready",
            "READY": "idle",
            "IDLE": "idle",
            "PAUSED": "paused",
            "STOPPED": "stopped",
            "ERROR": "error",
        }

        return state_map.get(seqstate, seqstate.lower() if seqstate else "stopped")



class ZmqStatusServiceThread(QThread):
    def __init__(self, zmq_status_service):
        super().__init__()
        self.zmq_status_service = zmq_status_service

    def run(self):
        # Start listening for messages in the StatusService
        self.zmq_status_service.logger.info("Starting ZMQStatusService thread...")
        self.zmq_status_service.listen()
