import socket
import threading
import time
from PyQt5.QtCore import pyqtSignal, QObject
import struct
import re  # To use regular expressions for parsing PIXELCOUNT

class StatusService(QObject, threading.Thread):
    # Signal to communicate with the main GUI thread
    status_updated_signal = pyqtSignal(str)
    progress_updated_signal = pyqtSignal(int)  # Signal to update exposure progress bar (0-100)
    readout_progress_updated_signal = pyqtSignal(int)  # Signal to update readout progress bar (0-100)

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
        """Creates a UDP socket and joins the multicast group."""
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(self.heartbeat_timeout)  # Timeout for receiving responses (heartbeat timeout)
        
        # Allow the socket to reuse the address
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        # Bind to the specified port (and default address 0.0.0.0 for all interfaces)
        self.sock.bind(("", self.port))
        
        # Set the socket option to join the multicast group
        mreq = struct.pack("4s4s", socket.inet_aton(self.ip), socket.inet_aton("0.0.0.0"))
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

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
                # Use select to check if there's data available to read
                ready_to_read, _, _ = select.select([self.sock], [], [], self.heartbeat_timeout)
                
                if ready_to_read:
                    # Read data if there's something available
                    data, addr = self.sock.recvfrom(1024)  # Buffer size of 1024 bytes
                    message = data.decode("utf-8")
                    
                    # Check if the message is of the format "EXPTIME"
                    if message.startswith("EXPTIME"):
                        self._parse_exptime_message(message)
                    elif message.startswith("PIXELCOUNT"):
                        self._parse_pixelcount_message(message)
                    else:
                        self.status = message  # Update with general status
                        self.heartbeat_misses = 0  # Reset heartbeat misses on successful response

                else:
                    # No data received within the timeout
                    self.heartbeat_misses += 1
                    self.status = "No response (Timeout)"
                    
                    # If we exceed the max misses, mark the service as disconnected
                    if self.heartbeat_misses >= self.max_heartbeat_misses:
                        self.status = "Heartbeat lost - Service Disconnected"
                        self.heartbeat_misses = 0  # Reset the counter after handling the issue

            except Exception as e:
                # If there's an exception, update status to show the error
                self.status = f"Error: {str(e)}"
            
            # Emit the updated status to the GUI
            self.status_updated_signal.emit(self.status)

            time.sleep(self.update_interval)  # Wait before listening for the next message

        # Close the socket when the thread stops
        self.sock.close()

    def _parse_exptime_message(self, message):
        """Parse EXPTIME message and update the exposure progress."""
        match = re.match(r"EXPTIME:(\d+) (\d+) (\d+)", message)
        if match:
            exposure_time = int(match.group(1))
            max_time = int(match.group(2))
            progress = int(match.group(3))

            # Calculate the progress as a percentage
            if max_time > 0:
                progress_percentage = (progress / max_time) * 100
                progress_percentage = min(max(progress_percentage, 0), 100)  # Clamp between 0 and 100
                self.progress_updated_signal.emit(int(progress_percentage))  # Send the progress update signal

    def _parse_pixelcount_message(self, message):
        """Parse PIXELCOUNT message and update the readout progress."""
        match = re.match(r"PIXELCOUNT_\d+:(\d+) (\d+) (\d+)", message)
        if match:
            current_count = int(match.group(1))
            total_count = int(match.group(2))
            progress = int(match.group(3))

            # Calculate the progress as a percentage
            if total_count > 0:
                progress_percentage = (current_count / total_count) * 100
                progress_percentage = min(max(progress_percentage, 0), 100)  # Clamp between 0 and 100
                self.readout_progress_updated_signal.emit(int(progress_percentage))  # Send the readout progress signal
