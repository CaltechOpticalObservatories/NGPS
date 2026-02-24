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
            process = await asyncio.create_subprocess_shell(
                command,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE,
            )

            # Stream stdout
            while True:
                line = await process.stdout.readline()
                if not line:
                    break
                decoded = line.decode().rstrip()
                self.output_signal.emit(decoded)
                self.log_message(decoded)

            # Stream stderr
            while True:
                err_line = await process.stderr.readline()
                if not err_line:
                    break
                decoded_err = err_line.decode().rstrip()
                self.output_signal.emit(f"[stderr] {decoded_err}")
                self.log_message(f"[stderr] {decoded_err}")

            await process.wait()
            if process.returncode == 0:
                self.output_signal.emit("Command finished successfully.")
            else:
                self.output_signal.emit(f"Command exited with code {process.returncode}")

        except Exception as e:
            self.output_signal.emit(f"Error running command: {str(e)}")
