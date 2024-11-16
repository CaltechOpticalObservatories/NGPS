# ngps_model.py

import subprocess
import shutil

class NgpsModel:
    def __init__(self):
        pass

    def is_ngps_available(self):
        """Check if the 'ngps' command is available in the system."""
        return shutil.which("ngps") is not None

    def execute_command(self, cmd):
        """Execute the command in the system shell and return the output."""
        try:
            result = subprocess.run([cmd], capture_output=True, text=True, check=True)
            return result.stdout, None  # Return output and no error
        except subprocess.CalledProcessError as e:
            return None, e.stderr  # Return no output and the error message

    def get_status(self):
        """Fetch the NGPS status from the system."""
        try:
            result = subprocess.run(['ngps', 'status'], capture_output=True, text=True, check=True)
            return result.stdout, None
        except subprocess.CalledProcessError as e:
            return None, e.stderr

    def reset_daemon(self):
        """Reset the NGPS daemon."""
        try:
            subprocess.run(['ngps', 'reset'], capture_output=True, text=True, check=True)
            return True, None  # Success
        except subprocess.CalledProcessError as e:
            return False, e.stderr  # Failure

