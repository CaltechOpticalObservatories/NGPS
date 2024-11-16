# ngps_viewmodel.py

from ngps_model import NgpsModel

class NgpsViewModel:
    def __init__(self):
        self.model = NgpsModel()

    def check_ngps_status(self):
        """Check if 'ngps' is available."""
        return self.model.is_ngps_available()

    def execute_command(self, cmd):
        """Execute a command and return the result or error."""
        output, error = self.model.execute_command(cmd)
        if error:
            return f"Error executing {cmd}: {error}"
        return f"Command '{cmd}' executed successfully.\n{output}"

    def fetch_ngps_status(self):
        """Fetch the NGPS status."""
        output, error = self.model.get_status()
        if error:
            return f"Error fetching NGPS status: {error}"
        return output

    def reset_ngps_daemon(self):
        """Reset the NGPS daemon."""
        success, error = self.model.reset_daemon()
        if error:
            return f"Error resetting daemon: {error}"
        return "Daemon reset successfully."

