import configparser
import socket
import logging

class SequencerService:
    def __init__(self, config_file):
        self.config = self.load_config(config_file)

        # Basic parameters from the config
        self.server_name = self.config.get('SERVERNAME', 'localhost')  # Default to 'localhost'
        self.command_server_port = self.config.getint('COMMAND_SERVERPORT', 8000)  # Default to 8000
        self.blocking_server_port = self.config.getint('BLOCKING_SERVERPORT', 9000)  # Default to 9000
        self.async_host = self.config.get('ASYNC_HOST', '239.1.1.234')  # Default to '239.1.1.234'
        self.async_server_port = self.config.getint('ASYNC_SERVERPORT', 1300)  # Default to 1300

        # Logging setup
        self.log_directory = self.config.get('LOG_DIRECTORY', '/data/logs')
        self.setup_logging()

        # Sockets to hold the connections
        self.command_socket = None
        self.blocking_socket = None
        self.async_socket = None

    def load_config(self, config_file):
        """ Loads the configuration file """
        config = configparser.ConfigParser()
        config.read(config_file)
        return config

    def setup_logging(self):
        """ Setup logging for the sequencer service """
        log_file = f"{self.log_directory}/sequencer_service.log"
        logging.basicConfig(filename=log_file, level=logging.INFO,
                            format='%(asctime)s - %(levelname)s - %(message)s')
        logging.info("SequencerService initialized")

    def connect(self):
        """ Establish the socket connections """
        try:
            # Connect to the command server
            self.command_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.command_socket.connect((self.server_name, self.command_server_port))
            logging.info(f"Connected to command server at {self.server_name}:{self.command_server_port}")

            # Connect to the blocking server (for synchronous requests)
            self.blocking_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.blocking_socket.connect((self.server_name, self.blocking_server_port))
            logging.info(f"Connected to blocking server at {self.server_name}:{self.blocking_server_port}")

            # Connect to the async server (for asynchronous requests)
            self.async_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.async_socket.connect((self.async_host, self.async_server_port))
            logging.info(f"Connected to async server at {self.async_host}:{self.async_server_port}")

        except Exception as e:
            logging.error(f"Failed to connect to one or more servers: {e}")
            raise e

    def disconnect(self):
        """ Close all active connections """
        try:
            if self.command_socket:
                self.command_socket.close()
                logging.info("Disconnected from command server.")
            if self.blocking_socket:
                self.blocking_socket.close()
                logging.info("Disconnected from blocking server.")
            if self.async_socket:
                self.async_socket.close()
                logging.info("Disconnected from async server.")
        except Exception as e:
            logging.error(f"Error while disconnecting: {e}")

    def send_command(self, command):
        """ Send a command to the command server """
        if self.command_socket:
            self.command_socket.sendall(command.encode('utf-8'))
            logging.info(f"Sent command: {command}")
        else:
            logging.error("No connection to command server.")

    def receive_response(self):
        """ Receive a response from the command server """
        if self.command_socket:
            response = self.command_socket.recv(1024).decode('utf-8')
            logging.info(f"Received response: {response}")
            return response
        else:
            logging.error("No connection to command server.")
            return None
