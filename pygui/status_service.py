import zmq
import logging
from PyQt5.QtCore import pyqtSignal, QObject, QThread

class StatusService(QObject):
    # Signal to send a new message
    new_message_signal = pyqtSignal(str)

    def __init__(self, parent, broker_publish_endpoint="tcp://127.0.0.1:5556"):
        super().__init__()
        self.parent = parent  # Reference to the parent window or main UI
        self.broker_publish_endpoint = broker_publish_endpoint
        self.context = zmq.Context()  # Create the ZeroMQ context
        self.socket = None
        self.is_connected = False

        # Set up logging
        self.setup_logging()
        print("StatusService initialized.")

    def setup_logging(self):
        """ Set up logging for the status service. """
        logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

    def connect(self):
        """ Connect to the broker using the SUB socket (not XSUB). """
        try:
            print(f"Connecting to broker at {self.broker_publish_endpoint}...")
            # Create the SUB socket type to receive messages
            self.socket = self.context.socket(zmq.SUB)
            self.socket.connect(self.broker_publish_endpoint)  # Connect to the broker (publisher's address and port)
            self.socket.setsockopt_string(zmq.SUBSCRIBE, "")  # Subscribe to all messages (default behavior)
            self.is_connected = True
            print(f"Connected to broker at {self.broker_publish_endpoint}")
        except Exception as e:
            print(f"Failed to connect to broker: {e}")
            self.is_connected = False
            raise e

    def subscribe(self, topic=None):
        """ Subscribe to a specific topic. If no topic is provided, subscribe to all topics. """
        if not self.is_connected:
            print("Not connected to broker. Call 'connect()' first.")
            return
        
        # If topic is provided, subscribe to that topic
        if topic:
            self.socket.setsockopt_string(zmq.SUBSCRIBE, topic)
            print(f"Subscribed to topic: {topic}")
        else:
            # If no topic is specified, subscribe to all messages
            self.socket.setsockopt_string(zmq.SUBSCRIBE, "")
            print("Subscribed to all topics.")

    def listen(self):
        """ Listen for incoming messages from the broker. """
        if not self.is_connected:
            print("Not connected to broker. Call 'connect()' first.")
            return

        try:
            print("Starting to listen for messages from the broker...")
            while True:
                message = self.socket.recv_string()  # Receive the message as a string
                print(f"Received message: {message}")
                
                # Process the message (split the topic and payload)
                topic, payload = message.split(' ', 1)  # Split the message assuming space-separated topic and payload
                
                print(f"Processed message: Topic = {topic}, Payload = {payload}")
                
                # Emit the message to the UI thread
                self.new_message_signal.emit(f"Topic: {topic}, Payload: {payload}")
        except Exception as e:
            print(f"Error while listening for messages: {e}")
        finally:
            self.disconnect()

    def disconnect(self):
        """ Disconnect from the broker and close the socket. """
        if self.socket:
            self.socket.close()
            self.is_connected = False
            print("Disconnected from broker.")

class StatusServiceThread(QThread):
    def __init__(self, status_service):
        super().__init__()
        self.status_service = status_service

    def run(self):
        # Start listening for messages in the StatusService
        print("Starting StatusService thread...")
        self.status_service.listen()
