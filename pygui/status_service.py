import socket
import threading
import time
import struct
from PyQt5.QtCore import pyqtSignal, QObject


class StatusService(QObject, threading.Thread):
    # Signal to send updates to the GUI
    status_updated_signal = pyqtSignal(str)

    def __init__(self, group="239.1.1.234", port=1300, update_interval=5, heartbeat_timeout=3, max_heartbeat_misses=3):
        QObject.__init__(self)
        threading.Thread.__init__(self)
        
        self.group = group
        self.port = port
        self.update_interval = update_interval
        self.heartbeat_timeout = heartbeat_timeout
        self.max_heartbeat_misses = max_heartbeat_misses
        self.sock = None
        self.status = "Waiting for status"
        self.heartbeat_misses = 0  # Counter for missed heartbeats
        self.running = True
        self.daemon = True  # Allow the thread to terminate when the main program exits

    def _create_socket(self):
        """Create and configure the socket for multicast."""
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, struct.pack('b', 255))  # TTL set to 255 (global scope)
        self.sock.settimeout(self.heartbeat_timeout)

    def stop(self):
        """Stop the status service thread."""
        self.running = False

    def run(self):
        """Start the multicast listening in a separate thread."""
        self._create_socket()

        # Bind to the port (to receive multicast messages on this port)
        self.sock.bind(('', self.port))

        # Join the multicast group
        mreq = struct.pack('4s4s', socket.inet_aton(self.group), socket.inet_aton('0.0.0.0'))
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

        while self.running:
            try:
                # Wait for incoming multicast packets
                data, addr = self.sock.recvfrom(1024)  # Buffer size of 1024 bytes
                self.status = data.decode("utf-8")
                self.heartbeat_misses = 0  # Reset heartbeat misses on successful response

            except socket.timeout:
                # Timeout indicates missed heartbeat
                self.heartbeat_misses += 1
                self.status = "No response (Timeout)"
                
                # If we exceed max missed heartbeats, indicate loss of connection
                if self.heartbeat_misses >= self.max_heartbeat_misses:
                    self.status = "Heartbeat lost - Service Disconnected"
                    self.heartbeat_misses = 0  # Reset after handling the issue

            except Exception as e:
                self.status = f"Error: {str(e)}"

            # Emit the updated status to the GUI
            self.status_updated_signal.emit(self.status)

            time.sleep(self.update_interval)  # Sleep for the defined interval

        # Clean up: Leave the multicast group and close the socket
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_DROP_MEMBERSHIP, mreq)
        self.sock.close()
