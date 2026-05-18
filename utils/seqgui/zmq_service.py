import zmq
import json
from PyQt5.QtCore import pyqtSignal, QObject, QThread

TOPIC_SEQ_SEQSTATE    = "seq_seqstate"
TOPIC_SEQ_WAITSTATE   = "seq_waitstate"
TOPIC_SEQ_DAEMONSTATE = "seq_daemonstate"
TOPIC_ACAMD           = "acamd"
TOPIC_SLICECAMD       = "slicecamd"
TOPIC_BROADCAST       = "broadcast"
TOPIC_CAMERAD         = "camerad"
TOPIC_SNAPSHOT        = "_snapshot"

KEY_SEQSTATE           = "seqstate"
KEY_BROADCAST_SEVERITY = "severity"
KEY_BROADCAST_MESSAGE  = "message"
KEY_SOURCE             = "source"


class SeqguiZmqService(QObject):
    # Signals to communicate with the main GUI thread
    seqstate_changed    = pyqtSignal(str)
    waitstate_changed   = pyqtSignal(dict)
    daemonstate_changed = pyqtSignal(dict)
    acamd_changed       = pyqtSignal(dict)
    slicecamd_changed   = pyqtSignal(dict)
    camerad_changed     = pyqtSignal(dict)
    sequencerd_alive    = pyqtSignal()
    broadcast_received  = pyqtSignal(str, str, str)  # severity, source, message
    connection_error    = pyqtSignal(str)

    def __init__(self, parent, sub_endpoint="tcp://localhost:5556", pub_endpoint="tcp://localhost:5555"):
        super().__init__()
        self.parent = parent  # Reference to the parent window or main UI
        self.sub_endpoint = sub_endpoint
        self.pub_endpoint = pub_endpoint
        self.context = zmq.Context.instance()  # Shared ZeroMQ context
        self.sub_socket = None
        self.pub_socket = None
        self.is_connected = False
        self.subscribed_topics = set()  # Set of subscribed topics
        self.running = True

    def connect(self):
        """ Connect SUB and PUB sockets to the broker. """
        try:
            self.sub_socket = self.context.socket(zmq.SUB)
            self.sub_socket.connect(self.sub_endpoint)
            self.pub_socket = self.context.socket(zmq.PUB)
            self.pub_socket.connect(self.pub_endpoint)
            self.is_connected = True
        except zmq.ZMQError as e:
            self.is_connected = False
            self.connection_error.emit(f"ZMQ connect failed: {e}")
            raise e

    def subscribe_to_topic(self, topic):
        """ Subscribe to a specific topic. """
        if not self.is_connected:
            return

        if topic not in self.subscribed_topics:
            self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, topic)
            self.subscribed_topics.add(topic)

    def stop(self):
        """ Request a clean shutdown of the listen loop. """
        self.running = False

    def listen(self):
        """ Listen for incoming messages from the broker. """
        if not self.is_connected:
            return

        # Brief sleep for the ZMQ slow-joiner: messages published immediately
        # after connect are dropped until the broker registers the subscriber.
        QThread.msleep(200)
        self._request_snapshot()

        poller = zmq.Poller()
        poller.register(self.sub_socket, zmq.POLLIN)

        try:
            while self.running:
                try:
                    events = dict(poller.poll(timeout=500))
                except zmq.ZMQError:
                    break
                if self.sub_socket not in events:
                    continue
                try:
                    parts = self.sub_socket.recv_multipart(flags=zmq.NOBLOCK)
                except zmq.Again:
                    continue
                if len(parts) < 2:
                    continue
                topic   = parts[0].decode("utf-8", errors="replace")
                payload = parts[1].decode("utf-8", errors="replace")

                try:
                    data = json.loads(payload)
                except json.JSONDecodeError:
                    continue
                if not isinstance(data, dict):
                    continue

                self._dispatch(topic, data)
        finally:
            self.disconnect()

    def disconnect(self):
        """ Disconnect from the broker and close the sockets. """
        if self.sub_socket:
            self.sub_socket.close(linger=0)
            self.sub_socket = None
        if self.pub_socket:
            self.pub_socket.close(linger=0)
            self.pub_socket = None
        self.is_connected = False

    def _request_snapshot(self):
        """ Publish a snapshot request so daemons re-emit current telemetry. """
        payload = json.dumps({"sequencerd": True, "acamd": True, "slicecamd": True, "camerad": True})
        try:
            self.pub_socket.send_multipart([TOPIC_SNAPSHOT.encode("utf-8"),
                                            payload.encode("utf-8")])
        except zmq.ZMQError as e:
            self.connection_error.emit(f"snapshot request failed: {e}")

    def _dispatch(self, topic, data):
        """ Emit the appropriate signal for the given topic and parsed payload. """
        if topic == TOPIC_SEQ_SEQSTATE:
            self.sequencerd_alive.emit()
            if KEY_SEQSTATE in data:
                self.seqstate_changed.emit(str(data[KEY_SEQSTATE]))
        elif topic == TOPIC_SEQ_WAITSTATE:
            self.waitstate_changed.emit(
                {k: bool(v) for k, v in data.items() if isinstance(v, bool)}
            )
        elif topic == TOPIC_SEQ_DAEMONSTATE:
            self.daemonstate_changed.emit(
                {k: bool(v) for k, v in data.items() if isinstance(v, bool)}
            )
        elif topic == TOPIC_ACAMD:
            self.acamd_changed.emit(data)
        elif topic == TOPIC_SLICECAMD:
            self.slicecamd_changed.emit(data)
        elif topic == TOPIC_CAMERAD:
            self.camerad_changed.emit(data)
        elif topic == TOPIC_BROADCAST:
            self.broadcast_received.emit(
                str(data.get(KEY_BROADCAST_SEVERITY, "")),
                str(data.get(KEY_SOURCE, "unknown")),
                str(data.get(KEY_BROADCAST_MESSAGE, "")),
            )


class SeqguiZmqServiceThread(QThread):
    def __init__(self, zmq_service):
        super().__init__()
        self.zmq_service = zmq_service

    def run(self):
        # Start listening for messages in the SeqguiZmqService
        self.zmq_service.listen()
