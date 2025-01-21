import socket
import threading
import time
from PyQt5.QtCore import pyqtSignal, QObject, QTimer
from PyQt5.QtWidgets import QMessageBox
import struct
import re
import select

class StatusService(QObject):
    # Signals to communicate with the main GUI thread
    status_updated_signal = pyqtSignal(str)
    progress_updated_signal = pyqtSignal(int)  # Signal to update exposure progress bar (0-100)
    readout_progress_updated_signal = pyqtSignal(int)  # Signal to update readout progress bar (0-100)

    def __init__(self, parent, ip="239.1.1.234", port=1300, update_interval=5, heartbeat_timeout=3, max_heartbeat_misses=3, timeout_duration=120):
        super().__init__()
        self.parent = parent  # reference to the parent window or main UI
        self.ip = ip
        self.port = port
        self.update_interval = update_interval
        self.heartbeat_timeout = heartbeat_timeout
        self.max_heartbeat_misses = max_heartbeat_misses
        self.timeout_duration = timeout_duration
        self.sock = None
        self.status = "Waiting for status"
        self.heartbeat_misses = 0
        self.running = False
        self.last_message = None
        self.last_emitted_message = None
        self.last_received_time = time.time()

        # Worker thread for communication
        self.thread = threading.Thread(target=self.run)
        self.thread.daemon = True
        self.timer = QTimer(self)  # Timer for periodic timeout checks
        self.timer.setInterval(self.update_interval * 1000)  # Set interval in milliseconds
        self.timer.timeout.connect(self.check_timeout)  # Connect to timeout check function

    def _create_socket(self):
        """Creates a UDP socket and joins the multicast group."""
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(self.heartbeat_timeout)  # Timeout for receiving responses (heartbeat timeout)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(("", self.port))

        mreq = struct.pack("4s4s", socket.inet_aton(self.ip), socket.inet_aton("0.0.0.0"))
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    def start(self):
        """Start the status service thread."""
        self.running = True
        self.thread.start()
        self.timer.start()  # Start the timer for periodic checks

    def stop(self):
        """Stop the status service thread."""
        self.running = False
        if self.thread.is_alive():
            self.thread.join()
        self.timer.stop()  # Stop the periodic timer

    def get_status(self):
        """Return the most recent status."""
        return self.status

    def run(self):
        """Run the status service in a separate thread."""
        self._create_socket()

        while self.running:
            try:
                ready_to_read, _, _ = select.select([self.sock], [], [], self.heartbeat_timeout)
                if ready_to_read:
                    data, addr = self.sock.recvfrom(1024)
                    message = data.decode("utf-8")
                    self.last_received_time = time.time()

                    if message != self.last_message:
                        self.last_message = message
                        self._handle_message(message)

            except Exception as e:
                self.status = f"Error: {str(e)}"
                self.heartbeat_misses += 1

            # No sleep here to avoid delays, just check for timeouts
            self.check_timeout()

    def check_timeout(self):
        """Check for timeouts and update status accordingly."""
        if time.time() - self.last_received_time >= self.timeout_duration:
            self.status = "No response (Timeout)"
            self.heartbeat_misses += 1
            if self.heartbeat_misses >= self.max_heartbeat_misses:
                self.status = "Heartbeat lost - Service Disconnected"
                self.heartbeat_misses = 0

        # Emit the updated status to the GUI if it's a new message
        if self.status != self.last_emitted_message:
            self.status_updated_signal.emit(self.status)
            self.last_emitted_message = self.status

    def _handle_message(self, message):
        """Handle the incoming message and decide what to do with it."""
        if "RUNSTATE: READY" in message:
            self.parent.show_popup("NGPS is Ready.")
            self.parent.layout_service.update_system_status("idle")
        elif message.startswith("EXPTIME"):
            self._parse_exptime_message(message)
        elif message.startswith("PIXELCOUNT"):
            self._parse_pixelcount_message(message)
        else:
            self.status = message
            self.heartbeat_misses = 0

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
                progress_percentage = min(max(progress_percentage, 0), 100)
                self.progress_updated_signal.emit(int(progress_percentage))

    def _parse_pixelcount_message(self, message):
        """Parse PIXELCOUNT message and update the readout progress."""
        match = re.match(r"PIXELCOUNT_\d+:(\d+) (\d+) (\d+)", message)
        if match:
            current_count = int(match.group(1))
            total_count = int(match.group(2))
            progress = int(match.group(3))

            if total_count > 0:
                progress_percentage = (current_count / total_count) * 100
                progress_percentage = min(max(progress_percentage, 0), 100)
                self.readout_progress_updated_signal.emit(int(progress_percentage))
