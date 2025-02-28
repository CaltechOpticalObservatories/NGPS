import socket
import logging

class SequencerService:
    def __init__(self, parent):
        self.parent = parent
        
        # Hardcoded values for the server connection
        self.server_name = 'localhost'  # Hardcoded value
        self.command_server_port = 9000  # Hardcoded value
        self.async_host = '239.1.1.234'  # Hardcoded value
        self.async_server_port = 1300  # Hardcoded value
        self.basename = 'ngps_image'  # Hardcoded value
        self.log_directory = '/data/logs'  # Hardcoded value
        
        #self.setup_logging()

        # Placeholder for active connections
        self.command_socket = None
        self.async_socket = None

    def setup_logging(self):
        """ Set up logging for the sequencer service. """
        log_file = f"{self.log_directory}/sequencer_service.log"
        logging.basicConfig(filename=log_file, level=logging.INFO,
                            format='%(asctime)s - %(levelname)s - %(message)s')
        logging.info("SequencerService initialized")

    def connect(self):
        """ Establish connections to the sequencer backend. """
        try:
            # Connect to the command server (for synchronous requests)
            self.command_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.command_socket.connect((self.server_name, self.command_server_port))
            print(f"Connected to command server at {self.server_name}:{self.command_server_port}")

            # Connect to the async server (for asynchronous requests)
            self.async_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.async_socket.connect((self.async_host, self.async_server_port))
            logging.info(f"Connected to async server at {self.async_host}:{self.async_server_port}")

        except socket.error as e:
            # Handle socket connection errors
            logging.error(f"Failed to connect to one or more servers: {e}")
            print(f"Failed to connect to one or more servers: {e}")
            # Optionally, you can still continue operation if the connection isn't critical

        except Exception as e:
            # General exception handling (if there is any other error)
            logging.error(f"Unexpected error during connection: {e}")
            print(f"Unexpected error during connection: {e}")
            # Optionally, handle the failure (e.g., fallback behavior, retry, etc.)
        
        finally:
            # Optionally, inform that the connection attempt is done
            print("Connection attempt complete. Continuing with application.")

    def disconnect(self):
        """ Close all active connections. """
        try:
            if self.command_socket:
                self.command_socket.close()
                logging.info("Disconnected from command server.")
            # if self.async_socket:
            #     self.async_socket.close()
            #     logging.info("Disconnected from async server.")
        except Exception as e:
            print(f"Error while disconnecting: {e}")

    def send_command(self, command):
        """ Send a command to the sequencer via the command server. """
        try:
            if self.command_socket:
                self.command_socket.sendall(command.encode('utf-8'))
                print(f"Sent command: {command}")
            else:
                print("No connection to command server.")
        except (AttributeError, OSError, socket.error) as e:
            # Handle specific exceptions, like socket errors or others
            print(f"Error while sending command: {e}")

    def receive_response(self):
        """ Receive a response from the sequencer via the command server. """
        if self.command_socket:
            response = self.command_socket.recv(1024).decode('utf-8')
            print(f"Received response: {response}")
            return response
        else:
            print("No connection to command server.")
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
