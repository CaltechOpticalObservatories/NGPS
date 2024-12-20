from PyQt5.QtWidgets import QDialog, QVBoxLayout, QGroupBox, QGridLayout, QLabel, QDialogButtonBox
from PyQt5.QtGui import QColor, QIcon
from PyQt5.QtCore import QSize

class PowerStatusDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)  # Initialize the parent (QMainWindow) properly

        self.setWindowTitle("Power Status")
        self.setGeometry(100, 100, 300, 200)  # Set size of the dialog window

        layout = QVBoxLayout()

        # Add a section for rate lamps power status
        self.create_rate_lamps_power_section(layout)

        # Add the "Close" button
        buttons = QDialogButtonBox(QDialogButtonBox.Close)
        buttons.clicked.connect(self.accept)  # Close the dialog when clicked
        layout.addWidget(buttons)

        self.setLayout(layout)

    def create_rate_lamps_power_section(self, layout):
        """Create a section for the power status of rate lamps"""
        group_box = QGroupBox("Rate Lamps Power Status", self)
        group_box_layout = QGridLayout()

        # Define the lamps and their status (True for 'on', False for 'off')
        self.rate_lamps = [
            ("LAMPTHAR", False), ("LAMPFEAR", True), ("LAMPBLUC", False),
            ("LAMPREDC", False), ("LAMP_MOD", True)
        ]

        # Add the lamps to the grid layout
        row = 0
        col = 0
        max_cols = 2  # We want 2 columns (lamp name and status light)

        for lamp, status in self.rate_lamps:
            lamp_label = QLabel(f"{lamp}:", self)
            status_label = QLabel(self)
            status_label.setFixedSize(30, 30)  # Set fixed size for the status light (circle)
            status_label.setStyleSheet(self.get_light_style(status))  # Apply color based on status

            # Add lamp name and status light to the grid layout
            group_box_layout.addWidget(lamp_label, row, col)
            group_box_layout.addWidget(status_label, row, col + 1)

            # Move to the next row when the max number of columns is reached
            col += 2
            if col >= max_cols * 2:
                col = 0
                row += 1

        group_box.setLayout(group_box_layout)
        layout.addWidget(group_box)

    def get_light_style(self, status):
        """Get the light style for the rate lamps: green for on, red for off"""
        color = QColor(0, 255, 0) if status else QColor(255, 0, 0)  # Green for on, Red for off
        return f"background-color: {color.name()}; border-radius: 15px;"  # Rounded circle
