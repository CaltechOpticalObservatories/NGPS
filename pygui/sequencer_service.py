import configparser
import socket
import logging

class SequencerService:
    def __init__(self, config_file):
        self.config = self.load_config(config_file)

        # Initialize connection parameters
        self.server_name = self.config.get('CONNECTION_PARAMETERS', 'SERVERNAME')
        self.instrument_name = self.config.get('CONNECTION_PARAMETERS', 'INSTRUMENT_NAME')
        self.command_server_port = int(self.config.get('CONNECTION_PARAMETERS', 'COMMAND_SERVERPORT'))
        self.blocking_server_port = int(self.config.get('CONNECTION_PARAMETERS', 'BLOCKING_SERVERPORT'))
        self.async_host = self.config.get('CONNECTION_PARAMETERS', 'ASYNC_HOST')
        self.async_server_port = int(self.config.get('CONNECTION_PARAMETERS', 'ASYNC_SERVERPORT'))
        self.basename = self.config.get('CONNECTION_PARAMETERS', 'BASENAME')
        self.log_directory = self.config.get('CONNECTION_PARAMETERS', 'LOG_DIRECTORY')

        # Set up logging
        self.setup_logging()

        # Placeholder for active connections
        self.command_socket = None
        self.blocking_socket = None
        self.async_socket = None

    def load_config(self, config_file):
        """ Load the configuration file. """
        config = configparser.ConfigParser()
        config.read(config_file)
        return config

    def setup_logging(self):
        """ Set up logging for the sequencer service. """
        log_file = f"{self.log_directory}/sequencer_service.log"
        logging.basicConfig(filename=log_file, level=logging.INFO,
                            format='%(asctime)s - %(levelname)s - %(message)s')
        logging.info("SequencerService initialized")

    def connect(self):
        """ Establish connections to the sequencer backend. """
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
        """ Close all active connections. """
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
        """ Send a command to the sequencer via the command server. """
        if self.command_socket:
            self.command_socket.sendall(command.encode('utf-8'))
            logging.info(f"Sent command: {command}")
        else:
            logging.error("No connection to command server.")

    def receive_response(self):
        """ Receive a response from the sequencer via the command server. """
        if self.command_socket:
            response = self.command_socket.recv(1024).decode('utf-8')
            logging.info(f"Received response: {response}")
            return response
        else:
            logging.error("No connection to command server.")
            return None

    # Specific methods for each command
    def acam(self, *args):
        """ Handle the 'acam' command """
        command = self.build_command("acam", *args)
        self.send_command(command)
        return self.receive_response()

    def calib(self, *args):
        """ Handle the 'calib' command """
        command = self.build_command("calib", *args)
        self.send_command(command)
        return self.receive_response()

    def camera(self, *args):
        """ Handle the 'camera' command """
        command = self.build_command("camera", *args)
        self.send_command(command)
        return self.receive_response()

    def filter(self, *args):
        """ Handle the 'filter' command """
        command = self.build_command("filter", *args)
        self.send_command(command)
        return self.receive_response()

    def power(self, *args):
        """ Handle the 'power' command """
        command = self.build_command("power", *args)
        self.send_command(command)
        return self.receive_response()

    def slit(self, *args):
        """ Handle the 'slit' command """
        command = self.build_command("slit", *args)
        self.send_command(command)
        return self.receive_response()

    def tcs(self, *args):
        """ Handle the 'tcs' command """
        command = self.build_command("tcs", *args)
        self.send_command(command)
        return self.receive_response()

    def build_command(self, cmd, *args):
        """ Build the command string to send to the sequencer. """
        command = f"seq {{ {cmd} }} "  # Begin the command structure
        if args:
            command += " ".join(str(arg) for arg in args)  # Add arguments to the command
        return command