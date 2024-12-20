from PyQt5.QtGui import QColor

class InstrumentStatusService:
    def __init__(self, parent):
        self.parent = parent
        self.status = "stopped"  # Default status
        self.update_status(self.status)  # Set the initial status

    def update_status(self, new_status):
        """Update the system status and change the label and color rectangle."""
        self.status = new_status
        self.update_status_label()

    def update_status_label(self):
        """Update the status label and rectangle based on the current status."""
        status_map = {
            "stopped": QColor(169, 169, 169),  # Grey
            "idle": QColor(255, 255, 0),  # Yellow
            "paused": QColor(255, 165, 0),  # Orange
            "exposing": QColor(0, 255, 0),  # Green
        }

        # Get the color based on the status (default to grey if status is unknown)
        color = status_map.get(self.status, QColor(169, 169, 169))  # Default to grey

        try:
            # Ensure the label and rectangle widgets exist
            if self.parent.instrument_status_label and self.parent.status_color_rect:
                # Update the text of the label
                self.parent.instrument_status_label.setText(f"System Status: {self.status.capitalize()}")
                
                # Update the color of the status rectangle next to the label
                self.parent.status_color_rect.setStyleSheet(f"background-color: {color.name()};")
        except AttributeError as e:
            print(f"Error updating status: {e}")
