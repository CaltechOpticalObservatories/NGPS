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
    
    async def execute_command(self):
        """Runs the given command in the terminal asynchronously."""
        try:
            print(f"Running command: {self.command}")
            # Start the subprocess asynchronously
            process = await asyncio.create_subprocess_shell(
                self.command, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE
            )
            
            # Wait for the command to finish and get the output and errors
            stdout, stderr = await process.communicate()

            # If the process has an error output, print it
            if stderr:
                self.output_signal.emit(f"Error executing command: {stderr.decode()}")
            
            # Otherwise, print the output
            if stdout:
                self.output_signal.emit(f"Command output: {stdout.decode()}")

            # Check the returncode for success/failure
            if process.returncode != 0:
                self.output_signal.emit(f"Command failed with return code {process.returncode}")
        
        except Exception as e:
            self.output_signal.emit(f"Error executing command: {e}")