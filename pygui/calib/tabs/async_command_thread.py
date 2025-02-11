import asyncio
from PyQt5.QtCore import QThread, pyqtSignal

class AsyncCommandThread(QThread):
    # Define a signal to communicate back to the UI thread
    output_signal = pyqtSignal(str)
    
    def __init__(self, command, parent=None):
        super().__init__(parent)
        self.command = command
    
    def run(self):
        # Run the async function in an event loop in this thread
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        loop.run_until_complete(self.execute_command())
    
    def run_command_in_background(self, command):
        """Run the command in a background thread."""
        self.thread = AsyncCommandThread(command)
        self.thread.output_signal.connect(self.log_message)
        self.thread.start()