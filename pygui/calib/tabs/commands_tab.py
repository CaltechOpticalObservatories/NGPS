from PyQt5.QtWidgets import QWidget, QVBoxLayout, QTabWidget
from calib.tabs.power_tab import PowerTab
from calib.tabs.calib_tab import CalibTab  
from calib.tabs.focus_cmd_tab import FocusCmdTab

class CommandsTab(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        layout = QVBoxLayout()

        # Create a QTabWidget for the Commands
        commands_tab_widget = QTabWidget()

        # Create the Power Tab (inside Commands Tab)
        power_tab = PowerTab()
        commands_tab_widget.addTab(power_tab, "power")

        # Create the Calib Tab (inside Commands Tab)
        calib_tab = CalibTab()  # Create the CalibTab instance
        commands_tab_widget.addTab(calib_tab, "calib")
        
        focus_tab = FocusCmdTab()
        commands_tab_widget.addTab(focus_tab, "focus")

        # Add the tab widget to the layout
        layout.addWidget(commands_tab_widget)
        self.setLayout(layout)
