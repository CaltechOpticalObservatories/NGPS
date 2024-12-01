import zmq
import logging
from PyQt5.QtCore import pyqtSignal, QObject
from PyQt5.QtCore import QThread

class StatusService(QObject):
    # Signal to send a new message
    new_message_signal = pyqtSignal(str)

    def __init__(self, parent, broker_publish_endpoint="tcp://127.0.0.1:5556"):
        super().__init__()
        self.parent = parent  # reference to the parent window or main UI
        self.broker_publish_endpoint = broker_publish_endpoint
        self.context = zmq.Context()  # Create a new ZeroMQ context
        self.socket = None
        self.is_connected = False
        
        # Set up logging
        self.setup_logging()

    def setup_logging(self):
        """ Set up logging for the status service. """
        logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

    def connect(self):
        """ Connect to the broker using the XPUB socket. """
        try:
            # Create the XPUB socket
            self.socket = self.context.socket(zmq.XSUB)  # XPUB socket type
            self.socket.bind(self.broker_publish_endpoint)
            self.is_connected = True
            logging.info(f"Connected to broker at {self.broker_publish_endpoint}")
        except Exception as e:
            logging.error(f"Failed to connect to broker: {e}")
            self.is_connected = False
            raise e

    def subscribe(self, topic=None):
        """ Subscribe to a topic. If no topic is provided, subscribe to all topics. """
        if not self.is_connected:
            logging.error("Not connected to broker. Call 'connect()' first.")
            return
        
        # If topic is provided, subscribe to that topic
        if topic:
            self.socket.setsockopt_string(zmq.SUBSCRIBE, topic)
            logging.info(f"Subscribed to topic: {topic}")
        else:
            # If no topic is specified, subscribe to all messages
            self.socket.setsockopt_string(zmq.SUBSCRIBE, "")
            logging.info("Subscribed to all topics.")

    def listen(self):
        """ Listen for incoming messages from the broker. """
        if not self.is_connected:
            logging.error("Not connected to broker. Call 'connect()' first.")
            return

        try:
            while True:
                message = self.socket.recv_string()  # Receive messages from the broker
                logging.info(f"Received message: {message}")
                # Emit the message to the UI thread
                self.new_message_signal.emit(message)
        except KeyboardInterrupt:
            logging.info("Exiting listener...")
        finally:
            self.disconnect()

    def disconnect(self):
        """ Disconnect from the broker and close the socket. """
        if self.socket:
            self.socket.close()
            self.is_connected = False
            logging.info("Disconnected from broker.")

class StatusServiceThread(QThread):
    def __init__(self, status_service):
        super().__init__()
        self.status_service = status_service

    def run(self):
        # Start listening for messages in the StatusService
        self.status_service.listen()