import sys
import subprocess
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QFileDialog, QMessageBox, QDialog, QDesktopWidget, QHBoxLayout
from PyQt5.QtCore import Qt, pyqtSlot
from menu_service import MenuService
from logic_service import LogicService
from layout_service import LayoutService
from sequencer_service import SequencerService
from login_service import LoginDialog, CreateAccountDialog
from zmq_status_service import ZmqStatusService, ZmqStatusServiceThread
from status_service import StatusService
from calib.calibration import CalibrationGUI
from etc_popup import EtcPopup
from control_tab import ControlTab 

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
        self.current_bin_spect = None
        self.current_bin_spat = None
        self.num_of_exposures = None
        self.user_set_data = {}
        self.all_targets = None
        self.current_owner = None
        self.current_target_list_name = None
        
        # Login status flag
        self.logged_in = False

        # Initialize the UI
        self.init_ui()
        
        # Initialize the Calibration GUI
        self.calibration_gui = None  # Will hold the reference to CalibrationGUI instance
        
        # Check sequencer state on startup
        if self.is_sequencer_ready():
            print("Sequencer is READY.")
        else:
            print("Sequencer is NOT READY.")

        # Get screen resolution using QDesktopWidget
        screen_geometry = QDesktopWidget().availableGeometry()  # Get screen size (excluding taskbar)
        screen_width, screen_height = screen_geometry.width(), screen_geometry.height()

        # Set window size relative to screen size (e.g., 80% of screen size)
        window_width = int(screen_width * 0.8)  # Use 80% for the left side content
        window_height = int(screen_height * 0.8)  # Adjust height as well

        # Set the geometry (position + size)
        self.setGeometry(int(screen_width * 0.1), int(screen_height * 0.1), window_width, window_height)
        
        # Load and apply the QSS stylesheet
        self.load_stylesheet("styles.qss")

        # Show ControlTab as a separate window
        # self.show_control_tab()
        
        # Initialize services
        self.initialize_services()
     
        # self.on_login()

    def init_ui(self):
        # Set up Menu
        menubar = self.menuBar()
        menu_service = MenuService(self, menubar)
        menu_service.create_menus()

        # Set up Layout
        self.layout_service = LayoutService(self)
        main_layout = self.layout_service.create_layout()

        self.logic_service = LogicService(self)
        
        # Try to connect to the MySQL database
        self.connection = self.logic_service.connect_to_mysql("config/db_config.ini")

        # Add layout to central widget
        central_widget = QWidget()
        central_widget.setLayout(main_layout)

        # Create a QHBoxLayout to hold the main layout and control tab side-by-side
        main_window_layout = QHBoxLayout()

        # Add the central widget (main content) to the left side
        main_window_layout.addWidget(central_widget)

        # Create a QWidget to hold the layout and set it as the central widget
        central_widget = QWidget()
        central_widget.setLayout(main_window_layout)
        self.setCentralWidget(central_widget)

    def initialize_services(self):

        # Initialize the SequencerService
        self.sequencer_service = SequencerService(self)
        self.sequencer_service.connect()
        
        # Start the StatusService in a separate thread with heartbeat
        self.status_service = StatusService(self)
        self.status_service.status_updated_signal.connect(self.layout_service.update_message_log)
        self.status_service.start()
        
        self.status_service.progress_updated_signal.connect(self.layout_service.update_exposure_progress)
        self.status_service.readout_progress_updated_signal.connect(self.layout_service.update_readout_progress)
        self.status_service.image_number_updated_signal.connect(self.layout_service.update_image_number)
        self.status_service.image_name_updated_signal.connect(self.layout_service.update_image_name)
        self.status_service.update_status_signal.connect(self.layout_service.update_system_status)
        self.status_service.user_can_expose_signal.connect(self.layout_service.control_tab.enable_continue_and_offset_button)

        # Initialize the ZMQStatusService
        self.zmq_status_service = ZmqStatusService(self)
        self.zmq_status_service.connect()
        
        # # Start the ZMQStatusService in a separate thread
        self.zmq_status_service_thread = ZmqStatusServiceThread(self.zmq_status_service)
        self.zmq_status_service_thread.start()
        self.zmq_status_service.subscribe_to_topic("powerd")
        self.zmq_status_service.subscribe_to_topic("calibd")
        self.zmq_status_service.subscribe_to_topic("tcsd")
        self.zmq_status_service.subscribe_to_topic("acamd")
        self.zmq_status_service.subscribe_to_topic("slitd")

        # Connect the message_received signal from ZMQStatusService to the update_message_log slot
        self.zmq_status_service.new_message_signal.connect(self.layout_service.update_message_log)
        self.zmq_status_service.lamp_states_signal.connect(self.layout_service.update_lamps)
        self.zmq_status_service.modulator_states_signal.connect(self.layout_service.update_modulators)
        self.zmq_status_service.airmass_signal.connect(self.layout_service.update_airmass)
        self.zmq_status_service.slit_info_signal.connect(self.layout_service.update_slit_info_fields)


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
        self.login_dialog = LoginDialog(self, self.connection)

        # If the login is successful, load data from MySQL
        if self.login_dialog.exec_() == QDialog.Accepted:
            # Call the function to load data from MySQL
            self.load_mysql_data(self.login_dialog.all_targets)
            self.user_set_data = self.login_dialog.set_data
            self.current_owner = self.login_dialog.owner
            # After loading data, populate the target lists dropdown
            self.layout_service.load_target_lists(self.login_dialog.set_name)
            
            # if self.layout_service.control_tab.startup_shutdown_button.text() == "Startup":
            #     self.layout_service.control_tab.toggle_startup_shutdown()

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

    def reload_table(self):
        self.logic_service.fetch_and_update_target_list()

    def is_sequencer_ready(self):
        """Check if the sequencer state is READY."""
        try:
            # Run the seq state command and capture output
            result = subprocess.run(['seq', 'state'], capture_output=True, text=True, check=True)
            # Strip any extra spaces or newlines
            state = result.stdout.strip()  # Remove surrounding whitespace or newlines
            
            # Check if the output starts with "READY"
            if state.startswith('READY'):
                self.layout_service.control_tab.startup_shutdown_button.setText("Shutdown")
                self.layout_service.control_tab.startup_shutdown_button.setStyleSheet("""
                    QPushButton {
                        background-color: #000000;  /* Black for shutdown */
                        border: none;
                        color: white;
                        font-weight: bold;
                        padding: 5px 10px;  /* Reduced padding */
                    }
                    QPushButton:hover {
                        background-color: #333333;
                    }
                    QPushButton:pressed {
                        background-color: #555555;
                    }
                """)
                self.layout_service.update_system_status("idle")
                return True  # Indicate that the sequencer is ready
        except subprocess.CalledProcessError as e:
            print(f"Error checking sequencer state: {e}")
        return False  # If not READY or an error occurs, return False

    def open_calibration_gui(self):
        """Method to open the Calibration GUI"""
        if self.calibration_gui is None or not self.calibration_gui.isVisible():
            self.calibration_gui = CalibrationGUI()
            self.calibration_gui.show()
        else:
            self.calibration_gui.raise_()  # Brings the window to the front if already open
            self.calibration_gui.activateWindow()

    def open_etc_popup(self):
        """Opens the EtcPopup when the button is clicked."""
        self.etc_popup = EtcPopup(self)  # Pass the parent as the current MainWindow
        self.etc_popup.exec_()

    def show_popup(self, message):
        """Show a popup message on the screen."""
        # Create a QMessageBox
        msg_box = QMessageBox()
        msg_box.setIcon(QMessageBox.Information)
        msg_box.setWindowTitle("Status Update")
        msg_box.setText(message)
        msg_box.setStandardButtons(QMessageBox.Ok)
        msg_box.exec_()
            
    # def show_control_tab(self):
    #     """
    #     This method creates and shows the ControlTab as a separate, closable popup window.
    #     """
    #     control_window = ControlTab(self)
    #     control_window.setGeometry(1400, 100, 400, 600)
    #     # Show the ControlTab window as a popup
    #     control_window.show()  

    @pyqtSlot()
    def on_delete_target_list(self):
        target_list_name = self.target_list_name.currentText()
        if not target_list_name:
            QMessageBox.warning(self, "Error", "No target list selected.")
            return

        confirm = QMessageBox.question(
            self, "Confirm Delete",
            f"Are you sure you want to delete target list '{target_list_name}'?",
            QMessageBox.Yes | QMessageBox.No
        )

        if confirm == QMessageBox.Yes:
            deleted = self.logic_service.delete_target_list_by_name(target_list_name)

            if deleted > 0:
                # Clean up state
                self.all_targets = [
                    row for row in self.all_targets
                    if row.get("NAME") != target_list_name
                ]
                self.user_set_data = {
                    k: v for k, v in self.user_set_data.items() if v != target_list_name
                }

                self.target_list_name.removeItem(
                    self.target_list_name.currentIndex()
                )

                self.layout_service.target_list_display.clearContents()
                self.layout_service.target_list_display.setRowCount(0)

                QMessageBox.information(self, "Deleted", f"Target list '{target_list_name}' was deleted.")
            else:
                QMessageBox.warning(self, "Not Found", f"No target list named '{target_list_name}' was found.")




if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = NgpsGUI()
    window.show()
    sys.exit(app.exec_())

