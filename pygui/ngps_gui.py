import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QFileDialog, QMessageBox, QDialog
from PyQt5.QtCore import Qt
from menu_service import MenuService
from logic_service import LogicService
from layout_service import LayoutService
from instrument_status_service import InstrumentStatusService
from sequencer_service import SequencerService
from login_service import LoginDialog, CreateAccountDialog
from status_service import StatusService, StatusServiceThread

class NgpsGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("NGPS")
        self.setGeometry(100, 100, 1200, 800)
        self.current_observation_id = None
        self.current_ra = None
        self.current_dec = None
        self.current_offset_ra = None
        self.current_offset_dec = None
        self.user_set_data = {}
        self.all_targets = None
        
        # Login status flag
        self.logged_in = False

        # Initialize the UI
        self.init_ui()
        
        # Load and apply the QSS stylesheet
        self.load_stylesheet("styles.qss")

        # Initialize the InstrumentStatusService
        self.instrument_status_service = InstrumentStatusService(self)

        # Initialize the SequencerService
        self.sequencer_service = SequencerService(self)
        self.sequencer_service.connect()

        # Initialize the StatusService
        self.status_service = StatusService(self)
        self.status_service.connect()
        
        # Start the StatusService in a separate thread
        self.status_service_thread = StatusServiceThread(self.status_service)
        self.status_service_thread.start()

        # Subscribe to a specific topic (optional)
        self.status_service.subscribe()
        # Connect the message_received signal from StatusService to the update_message_log slot
        self.status_service.new_message_signal.connect(self.layout_service.update_message_log)

    def init_ui(self):
        # Set up Menu
        menubar = self.menuBar()
        menu_service = MenuService(self, menubar)
        menu_service.create_menus()


        # Initialize the message log
        self.message_log = None
        
        # Set up Layout
        self.layout_service = LayoutService(self)
        main_layout = self.layout_service.create_layout()

        self.logic_service = LogicService(self)
        
        # Try to connect to the MySQL database
        self.connection = self.logic_service.connect_to_mysql("config/db_config.ini")

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

    def load_csv_file(self):
        # Open file dialog to select a CSV file
        file_path, _ = QFileDialog.getOpenFileName(self, "Open CSV File", "", "CSV Files (*.csv)")

        if file_path:
            file_name_only = file_path.split("/")[-1]  # Get the file name only (without the path)
            # Update the target list combo box with the file name
            self.target_list_name.addItem(file_name_only)
        
            # Use LogicService to load CSV and update the table
            self.logic_service.load_csv_and_update_target_list(file_path)

    def on_login(self):
        """Handle the login action from the menu."""
        login_dialog = LoginDialog(self, self.connection)

        # If the login is successful, load data from MySQL
        if login_dialog.exec_() == QDialog.Accepted:
            # Call the function to load data from MySQL
            self.load_mysql_data(login_dialog.all_targets)
            self.user_set_data = login_dialog.set_data
            # After loading data, populate the target lists dropdown
            self.layout_service.load_target_lists(login_dialog.set_name)


    def load_mysql_data(self, all_targets):
        """Load data from MySQL after successful login."""
        # Pass the data (all_targets) and the selected set_name (target_list_name) to the update function
        self.all_targets = all_targets
        self.logic_service.update_target_list_table(all_targets)

    def on_create_account(self):
        """ Handle the create account action from the User menu """
        create_account_dialog = CreateAccountDialog(self)
        
        # If account creation is successful, handle it (e.g., show success message)
        if create_account_dialog.exec_() == QDialog.Accepted:
            print("Account successfully created!")

    def send_command(self, command):
        """ Load data from MySQL after successful login """
        self.sequencer_service.send_command(command)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = NgpsGUI()
    window.show()
    sys.exit(app.exec_())
