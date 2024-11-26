import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QWidget, QVBoxLayout, QHBoxLayout
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor
from menu_service import MenuService
from logic_service import LogicService
from layout_service import LayoutService
from instrument_status_service import InstrumentStatusService

class NgpsGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("NGPS")
        self.setGeometry(100, 100, 1200, 800)

        # Initialize the UI
        self.init_ui()

        # # Load and apply the QSS stylesheet
        self.load_stylesheet("styles.qss")

        # Initialize the InstrumentStatusService after UI setup
        self.instrument_status_service = InstrumentStatusService(self)

    def init_ui(self):
        # Set up Menu
        menubar = self.menuBar()
        menu_service = MenuService(menubar)
        menu_service.create_menus()

        # Set up Layout
        layout_service = LayoutService(self)
        main_layout = layout_service.create_layout()

        # Add layout to central widget
        central_widget = QWidget()
        central_widget.setLayout(main_layout)
        self.setCentralWidget(central_widget)

        # Connect the DateTimeEdit to the on_date_time_changed function
        self.start_date_time_edit.dateTimeChanged.connect(self.on_date_time_changed)

    def on_date_time_changed(self, datetime):
        start_time_utc = LogicService.convert_pst_to_utc(datetime)
        self.utc_start_time.setText(start_time_utc.strftime('%m/%d/%Y %I:%M %p UTC'))

    # Function to apply QSS file to the app
    def load_stylesheet(self, qss_file):
        with open(qss_file, "r") as f:
            qss = f.read()
            self.setStyleSheet(qss)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = NgpsGUI()
    window.show()
    sys.exit(app.exec_())
