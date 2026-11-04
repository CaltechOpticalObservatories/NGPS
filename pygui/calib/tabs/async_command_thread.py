import asyncio
from PyQt5.QtCore import QThread, pyqtSignal

class AsyncCommandThread(QThread):
    output_signal = pyqtSignal(str)
    
    def __init__(self, command, log_message_callback, parent=None):
        super().__init__(parent)
        self.command = command
        self.log_message = log_message_callback
    
    def run(self):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        loop.run_until_complete(self.run_command_in_background(self.command))
    
    async def run_command_in_background(self, command):
        """Run the command and stream output line-by-line."""
        try:
            # Merge stderr into stdout. Reading two pipes one after the other
            # deadlocks once the child fills the unread stderr pipe.
            process = await asyncio.create_subprocess_shell(
                command,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.STDOUT,
            )

            # Stream stdout
            while True:
                line = await process.stdout.readline()
                if not line:
                    break
                decoded = line.decode(errors="replace").rstrip()
                self.output_signal.emit(decoded)

            await process.wait()
            if process.returncode == 0:
                self.output_signal.emit("Command finished successfully.")
            else:
                self.output_signal.emit(f"Command exited with code {process.returncode}")

        except Exception as e:
            self.output_signal.emit(f"Error running command: {str(e)}")
