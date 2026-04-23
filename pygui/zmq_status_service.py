import zmq
import os
import logging
import json
from PyQt5.QtCore import pyqtSignal, QObject, QThread
from typing import Dict

class ZmqStatusService(QObject):
    # Signal to send a new message
    new_message_signal = pyqtSignal(str)

    # Signal to send lamp states as a dictionary {lamp_name: bool}
    lamp_states_signal = pyqtSignal(dict)
    
    # Signal to send modulator states as a dictionary {modulator_name: bool}
    modulator_states_signal = pyqtSignal(dict)

    airmass_signal = pyqtSignal(float) 
    
    slit_info_signal = pyqtSignal(float, float)
    
    system_status_signal = pyqtSignal(str)

    def __init__(self, parent, broker_publish_endpoint="tcp://127.0.0.1:5556"):
        super().__init__()
        self.parent = parent  # Reference to the parent window or main UI
        self.broker_publish_endpoint = broker_publish_endpoint
        self.context = zmq.Context()  # Create the ZeroMQ context
        self.socket = None
        self.is_connected = False
        self.subscribed_topics = set()  # Set of subscribed topics
        self._last_seq_lifecycle_status = "stopped"
        self._last_seq_wait_status = None

        # Set up logging
        self.setup_logging()
        self.logger.info("StatusService initialized.")

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


    def listen(self):
        """ Listen for incoming messages from the broker. """
        if not self.is_connected:
            self.logger.warning("Not connected to broker. Call 'connect()' first.")
            return

        try:
            self.logger.info("Starting to listen for messages from the broker...")
            while True:
                message = self.socket.recv_multipart()
                if len(message) == 2:
                    topic = message[0].decode("utf-8")
                    payload = message[1].decode("utf-8")

                    self.logger.info(f'Received message: Topic = {topic}, Payload = {payload}')

                    try:
                        data = json.loads(payload)

                        if topic == "acamd":
                            self.new_message_signal.emit(f"Topic: {topic}, Payload: {payload}")

                        elif topic == "seq_seqstate":
                            self._last_seq_lifecycle_status = self._status_from_seq_seqstate(data)
                            self._emit_resolved_system_status()

                        elif topic == "seq_waitstate":
                            self._last_seq_wait_status = self._status_from_seq_waitstate(data)
                            self._emit_resolved_system_status()

                        if topic == "slitd":
                            slit_width = data.get("SLITW", None)
                            slit_offset = data.get("SLITO", None)
                            if slit_width is not None and slit_offset is not None:
                                self.slit_info_signal.emit(slit_width, slit_offset)

                        if topic == "calibd":
                            self.new_message_signal.emit(f"Topic: {topic}, Payload: {payload}")
                            self.update_modulator_states(data)

                        if topic == "powerd":
                            self.new_message_signal.emit(f"Topic: {topic}, Payload: {payload}")
                            self.update_lamp_states(data)

                        if topic == "tcsd":
                            self.update_tcs_info(data)

                    except json.JSONDecodeError as e:
                        self.logger.error(f"Error parsing JSON payload: {e}")
                else:
                    self.logger.warning("Received malformed message (not two parts).")

        except Exception as e:
            self.logger.error(f"Error while listening for messages: {e}")
        finally:
            self.disconnect()

    def disconnect(self):
        """ Disconnect from the broker and close the socket. """
        if self.socket:
            self.socket.close()
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

    def _emit_resolved_system_status(self):
        """
        If a wait-state is active, show that.
        Otherwise show the broader sequencer lifecycle state.
        """
        status = self._last_seq_wait_status or self._last_seq_lifecycle_status
        self.system_status_signal.emit(status)

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
            "ERROR": "stopped",
        }

        return state_map.get(seqstate, seqstate.lower() if seqstate else "stopped")

    def _status_from_seq_waitstate(self, flags: Dict[str, Any]) -> Optional[str]:
        """
        Return the active wait-state if one is true, else None.
        Returning None falls back to seq_seqstate.
        """
        if not isinstance(flags, dict):
            return None

        # Ignore metadata fields like "source"
        f = {
            str(k).upper(): bool(v)
            for k, v in flags.items()
            if str(k).lower() != "source"
        }

        # Precedence matters if more than one field is true.
        # Put the most user-meaningful states first.
        wait_order = [
            ("READOUT", "readout"),
            ("EXPOSE", "exposing"),

            # New replacement states for old ACQUIRE
            ("MOVETO", "moveto"),
            ("ACAM_ACQUIRE", "acam_acquire"),
            ("SLICECAM_FINEACQUIRE", "slicecam_fineacquire"),

            ("FOCUS", "focus"),
            ("CALIB", "calib"),
            ("CAMERA", "camera"),
            ("FLEXURE", "flexure"),
            ("POWER", "power"),
            ("SLIT", "slit"),
            ("TCSOP", "tcsop"),
            ("TCS", "tcs"),
            ("USER", "user"),

            ("ACAM", "acam"),
            ("SLICECAM", "slicecam"),
            ("ACQUIRE", "acquire"),
        ]

        for key, ui_status in wait_order:
            if f.get(key, False):
                return ui_status

        return None

class ZmqStatusServiceThread(QThread):
    def __init__(self, zmq_status_service):
        super().__init__()
        self.zmq_status_service = zmq_status_service

    def run(self):
        # Start listening for messages in the StatusService
        self.zmq_status_service.logger.info("Starting ZMQStatusService thread...")
        self.zmq_status_service.listen()
