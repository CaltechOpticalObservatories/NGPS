import zmq
import os
import logging
from PyQt5.QtCore import pyqtSignal, QObject, QThread

class ZmqStatusService(QObject):
    # Signal to send a new message
    new_message_signal = pyqtSignal(str)

    def __init__(self, parent, broker_publish_endpoint="tcp://127.0.0.1:5556"):
        super().__init__()
        self.parent = parent  # Reference to the parent window or main UI
        self.broker_publish_endpoint = broker_publish_endpoint
        self.context = zmq.Context()  # Create the ZeroMQ context
        self.socket = None
        self.is_connected = False
        self.subscribed_topics = set()  # Set of subscribed topics

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
                message = self.socket.recv_string()  # Receive the message as a string
                self.logger.info(f"Received message: {message}")
                
                # Check if the message contains a space to separate topic and payload
                if ' ' in message:
                    topic, payload = message.split(' ', 1)  # Split the message assuming space-separated topic and payload
                    self.logger.info(f"Processed message: Topic = {topic}, Payload = {payload}")
                    # Emit the message to the UI thread
                    self.new_message_signal.emit(f"Topic: {topic}, Payload: {payload}")
                else:
                    # If there is no space, treat the whole message as the topic
                    self.logger.info(f"Processed message with no payload: Topic = {message}")
                    self.new_message_signal.emit(f"Topic: {message}, Payload: None")
                    
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

class ZmqStatusServiceThread(QThread):
    def __init__(self, zmq_status_service):
        super().__init__()
        self.zmq_status_service = zmq_status_service

    def run(self):
        # Start listening for messages in the StatusService
        self.zmq_status_service.logger.info("Starting ZMQStatusService thread...")
        self.zmq_status_service.listen()