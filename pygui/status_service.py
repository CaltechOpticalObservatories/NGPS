import socket
import threading
import time
from PyQt5.QtCore import pyqtSignal, QObject

class StatusService(QObject, threading.Thread):
    # Signal to communicate with the main GUI thread
    status_updated_signal = pyqtSignal(str)

    def __init__(self, ip="239.1.1.234", port=1300, update_interval=5, heartbeat_timeout=3, max_heartbeat_misses=3):
        threading.Thread.__init__(self)
        QObject.__init__(self)
        self.ip = ip
        self.port = port
        self.update_interval = update_interval
        self.heartbeat_timeout = heartbeat_timeout  # Timeout threshold in seconds
        self.max_heartbeat_misses = max_heartbeat_misses  # Maximum number of missed heartbeats
        self.sock = None
        self.status = "Waiting for status"
        self.heartbeat_misses = 0  # Counts missed heartbeats
        self.running = True
        self.daemon = True  # Allow thread to exit when the program ends
    
    def _create_socket(self):
        """Creates a UDP socket."""
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(self.heartbeat_timeout)  # Timeout for receiving responses (heartbeat timeout)
    
    def get_status(self):
        """Return the most recent status."""
        return self.status
    
    def stop(self):
        """Stop the status service thread."""
        self.running = False

    def run(self):
        """Run the status service in a separate thread."""
        self._create_socket()
        
        while self.running:
            try:
                # Send the status request (you can customize the message here)
                message = b"GET_STATUS"
                self.sock.sendto(message, (self.ip, self.port))
                
                # Wait for a response from the server
                data, addr = self.sock.recvfrom(1024)  # Buffer size of 1024 bytes
                self.status = data.decode("utf-8")
                self.heartbeat_misses = 0  # Reset heartbeat misses on successful response
                
            except socket.timeout:
                # If we time out, increment the heartbeat misses counter
                self.heartbeat_misses += 1
                self.status = "No response (Timeout)"
                
                # If we exceed the max misses, mark the service as disconnected
                if self.heartbeat_misses >= self.max_heartbeat_misses:
                    self.status = "Heartbeat lost - Service Disconnected"
                    self.heartbeat_misses = 0  # Reset the counter after handling the issue
            
            except Exception as e:
                self.status = f"Error: {str(e)}"
            
            # Emit the updated status to the GUI
            self.status_updated_signal.emit(self.status)

            time.sleep(self.update_interval)  # Wait before sending the next request

        # Close the socket when the thread stops
        self.sock.close()
