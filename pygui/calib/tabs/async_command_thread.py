import asyncio
import subprocess
from PyQt5.QtCore import QThread, pyqtSignal

class AsyncCommandThread(QThread):
    # Define a signal to communicate back to the UI thread
    output_signal = pyqtSignal(str)
    
    def __init__(self, command, log_message_callback, parent=None):
        super().__init__(parent)
        self.command = command
        self.log_message = log_message_callback
    
    def run(self):
        # Run the async function in an event loop in this thread
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        loop.run_until_complete(self.run_command_in_background(self.command))
    
    async def run_command_in_background(self, command):
        """Run the command asynchronously."""
        try:
            result = await self.execute_command(command)
            # Emit the result back to the main thread
            self.output_signal.emit(result)
        except Exception as e:
            # Emit error message back to the main thread if an exception occurs
            self.output_signal.emit(f"Error: {str(e)}")
    
    async def execute_command(self, command):
        """Execute the command asynchronously and get the result."""
        try:
            result = await asyncio.to_thread(subprocess.run, command, shell=True, capture_output=True, text=True)
            
            if result.returncode == 0:
                return f"Command executed successfully: {result.stdout.strip()}"
            else:
                return f"Command failed with error: {result.stderr.strip()}"
        except Exception as e:
            return f"Exception running command: {str(e)}"
