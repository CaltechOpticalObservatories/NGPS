from PyQt5.QtWidgets import QVBoxLayout, QFrame, QAbstractItemView,  QHBoxLayout, QTableWidget, QHeaderView, QFormLayout, QListWidget, QListWidgetItem, QScrollArea, QVBoxLayout, QGroupBox, QGroupBox, QHeaderView, QLabel, QRadioButton, QProgressBar, QLineEdit, QTextEdit, QTableWidget, QComboBox, QDateTimeEdit, QTabWidget, QWidget, QPushButton, QCheckBox,QSpacerItem, QSizePolicy
from PyQt5.QtCore import QDateTime, QTimer
from PyQt5.QtGui import QColor, QFont
from instrument_status_service import InstrumentStatusService
from logic_service import LogicService
from PyQt5.QtCore import Qt

class LayoutService:
    def __init__(self, parent):
        self.parent = parent
        self.instrument_status_service = InstrumentStatusService(self.parent)
        self.logic_service = LogicService(self.parent)
        self.target_list_display = None 

    def create_layout(self):
        main_layout = QHBoxLayout()

        # First Column (Target List) should take 3/5 of the width
        first_column_layout = self.create_first_column()
        main_layout.addLayout(first_column_layout, stretch=3)  # Target List section

        # Second Column (Top Section) should take 2/5 of the width
        second_column_layout = self.create_second_column()
        main_layout.addLayout(second_column_layout, stretch=2)  # Top Section

        # Third Column (1/5 width, for tabs, etc.)
        third_column_layout = self.create_third_column()
        main_layout.addLayout(third_column_layout, stretch=1)

        # Set the main layout
        central_widget = QWidget()
        self.parent.setCentralWidget(central_widget)
        central_widget.setLayout(main_layout)

        return main_layout

    def create_first_column(self):
        first_column_layout = QVBoxLayout()
        first_column_layout.setObjectName("column-left")
        first_column_layout.setSpacing(10)

        # Top Section: Instrument System Status, Sequencer Mode, Progress & Image Info
        top_section_layout = self.create_top_section()
        first_column_layout.addLayout(top_section_layout)

        # Bottom Section: Target List
        target_list_group = self.create_target_list_group()
        first_column_layout.addWidget(target_list_group)

        return first_column_layout

    def create_second_column(self):
        second_column_layout = QVBoxLayout()
        second_column_layout.setObjectName("column-right")
        second_column_layout.setSpacing(10)

        # Create the top half of the second column with tabs
        second_column_top_half = self.create_second_column_top_half()
        second_column_layout.addWidget(second_column_top_half)

        # Create the target information group (remaining part of the second column)
        target_info_group = self.create_target_info_group()
        second_column_layout.addWidget(target_info_group)

        return second_column_layout

    def create_third_column(self):
        third_column_layout = QVBoxLayout()

        # Create the QTabWidget and the Control tab
        self.parent.tabs = QTabWidget()
        self.parent.control_tab = QWidget()
        self.parent.status_tab = QWidget()
        self.parent.engineering_tab = QWidget()

        # Add the tabs to the QTabWidget
        self.parent.tabs.addTab(self.parent.control_tab, "Control")
        self.parent.tabs.addTab(self.parent.status_tab, "Status")
        self.parent.tabs.addTab(self.parent.engineering_tab, "Engineering")

        # Add the QTabWidget to the third column layout
        third_column_layout.addWidget(self.parent.tabs)

        # Create and set up the layout for the Control tab
        self.create_control_tab(self.parent.control_tab)

        return third_column_layout

    def create_top_section(self):
        top_section_layout = QHBoxLayout()
        top_section_layout.setSpacing(10)

        # Left side: Instrument System Status and Sequencer Mode
        left_top_layout = self.create_left_top_layout()
        top_section_layout.addLayout(left_top_layout)

        # Right side: Progress & Image Info
        right_top_layout = self.create_right_top_layout()
        top_section_layout.addLayout(right_top_layout)

        return top_section_layout

    def create_left_top_layout(self):
        left_top_layout = QVBoxLayout()
        left_top_layout.setSpacing(10)

        # Instrument System Status
        system_status_group = self.create_system_status_group()
        left_top_layout.addWidget(system_status_group)

        # Sequencer Mode
        sequencer_mode_group = self.create_sequencer_mode_group()
        left_top_layout.addWidget(sequencer_mode_group)

        return left_top_layout

    def create_right_top_layout(self):
        right_top_layout = QVBoxLayout()
        right_top_layout.setSpacing(10)

        # Progress and Image Info Group
        progress_and_image_group = self.create_progress_and_image_group()
        right_top_layout.addWidget(progress_and_image_group)

        return right_top_layout

    def create_system_status_group(self):
        system_status_group = QGroupBox("Instrument System Status")
        system_status_layout = QVBoxLayout()
        system_status_layout.setSpacing(5)

        self.parent.instrument_status_label = QLabel("System Status:")
        system_status_layout.addWidget(self.parent.instrument_status_label)

        status_list_layout = QVBoxLayout()
        status_map = {
            "stopped": QColor(169, 169, 169),  # Grey
            "idle": QColor(255, 255, 0),       # Yellow
            "paused": QColor(255, 165, 0),     # Orange
            "exposing": QColor(0, 255, 0),     # Green
        }

        for status, color in status_map.items():
            status_layout = QHBoxLayout()
            status_color_rect = QWidget()
            status_color_rect.setFixedSize(20, 20)
            status_color_rect.setStyleSheet(f"background-color: {color.name()};")
            status_label = QLabel(status.capitalize())
            status_layout.addWidget(status_color_rect)
            status_layout.addWidget(status_label)
            status_list_layout.addLayout(status_layout)

        system_status_group.setLayout(status_list_layout)

        # Set maximum width and height for the system status group
        system_status_group.setMaximumWidth(300)  # Maximum width
        system_status_group.setMaximumHeight(250)  # Maximum height

        return system_status_group


    def create_sequencer_mode_group(self):
        sequencer_mode_group = QGroupBox("Sequencer Mode")
        sequencer_mode_layout = QVBoxLayout()

        self.parent.sequencer_mode_single = QRadioButton("Single")
        self.parent.sequencer_mode_all = QRadioButton("All")
        sequencer_mode_layout.addWidget(self.parent.sequencer_mode_single)
        sequencer_mode_layout.addWidget(self.parent.sequencer_mode_all)

        sequencer_mode_group.setLayout(sequencer_mode_layout)

        # Set maximum width and height for the sequencer mode group
        sequencer_mode_group.setMaximumWidth(300)  # Maximum width
        sequencer_mode_group.setMaximumHeight(100)  # Maximum height

        return sequencer_mode_group

    def create_progress_and_image_group(self):
        progress_and_image_group = QGroupBox("Progress and Image Info")
        progress_and_image_layout = QVBoxLayout()
        progress_and_image_layout.setSpacing(10)

        progress_layout = self.create_progress_layout()
        image_info_layout = self.create_image_info_layout()
        message_log = self.create_message_log()

        progress_and_image_layout.addLayout(progress_layout)
        progress_and_image_layout.addLayout(image_info_layout)
        progress_and_image_layout.addWidget(QLabel("Log Messages:"))
        progress_and_image_layout.addWidget(message_log)

        progress_and_image_group.setLayout(progress_and_image_layout)

        # Set maximum width and height for the progress and image info group
        progress_and_image_group.setMaximumWidth(750)  # Maximum width
        progress_and_image_group.setMaximumHeight(350)  # Maximum height

        return progress_and_image_group


    def create_progress_layout(self):
        progress_layout = QHBoxLayout()
        progress_layout.setSpacing(10)

        self.parent.exposure_progress = QProgressBar()
        self.parent.exposure_progress.setRange(0, 100)
        self.parent.exposure_progress.setMaximumWidth(200)  # Max width for progress bar

        self.parent.overhead_progress = QProgressBar()
        self.parent.overhead_progress.setRange(0, 100)
        self.parent.overhead_progress.setMaximumWidth(220)  # Max width for progress bar

        progress_layout.addWidget(QLabel("Overhead Progress"))
        progress_layout.addWidget(self.parent.overhead_progress)
        progress_layout.addWidget(QLabel("Exposure Progress"))
        progress_layout.addWidget(self.parent.exposure_progress)

        return progress_layout


    def create_image_info_layout(self):
        image_info_layout = QHBoxLayout()
        image_info_layout.setSpacing(10)

        self.parent.image_name = QLineEdit("NGPS_2024_11_21_Image_1.fits")
        self.parent.image_number = QLineEdit("1")

        image_info_layout.addWidget(QLabel("Image Name:"))
        image_info_layout.addWidget(self.parent.image_name)
        image_info_layout.addWidget(QLabel("Image Number:"))
        image_info_layout.addWidget(self.parent.image_number)

        return image_info_layout

    def create_message_log(self):
        self.parent.message_log = QTextEdit(self.parent)
        
        # Set size policies to allow the widget to stretch and grow proportionally
        self.parent.message_log.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.parent.message_log.setReadOnly(True)

        # Optionally set a minimum height or width if desired (not fixed size)
        self.parent.message_log.setMinimumHeight(60)
        self.parent.message_log.setMinimumWidth(200)  # Set a reasonable minimum width

        return self.parent.message_log
    
    def update_message_log(self, message):
        """ Update the message log with the new message. """
        if self.parent.message_log:
            current_text = self.parent.message_log.toPlainText()
            updated_text = current_text + "\n" + message
            self.parent.message_log.setPlainText(updated_text)
            # Optionally, scroll to the bottom of the text log
            cursor = self.parent.message_log.textCursor()
            cursor.movePosition(cursor.End)
            self.parent.message_log.setTextCursor(cursor)

    def create_target_list_group(self):
        target_list_group = QGroupBox("Target List")
        bottom_section_layout = QVBoxLayout()
        bottom_section_layout.setSpacing(5)

        # Create the button to load the target list
        self.load_target_button = QPushButton("Please login or load your target list to start")
        self.load_target_button.setStyleSheet("""
            QPushButton {
                background-color: #FFCC40;
                border: none;
                color: black;
                font-weight: bold;
                padding: 10px;
            }
            QPushButton:hover {
                background-color: #FF9900;
            }
            QPushButton:pressed {
                background-color: #FF6600;
            }
        """)

        # Center the button by using a QHBoxLayout
        button_layout = QHBoxLayout()
        button_layout.addWidget(self.load_target_button)
        button_layout.setAlignment(self.load_target_button, Qt.AlignCenter)

        self.load_target_button.clicked.connect(self.parent.load_csv_file)  # Connect to load CSV functionality
        bottom_section_layout.addWidget(self.load_target_button)

        # Create the QTableWidget for the target list
        self.target_list_display = QTableWidget()
        self.target_list_display.setRowCount(0)  # Set to 0 initially
        self.target_list_display.setColumnCount(0)  # Set column count to 0 initially

        # Create a placeholder column for the target data
        self.target_list_display.setHorizontalHeaderLabels([])  # Initially no headers

        # Remove the bold font from headers
        header = self.target_list_display.horizontalHeader()
        header.setFont(QFont("Arial", 10, QFont.Normal))  # Set font to normal (non-bold)

        # Enable horizontal scrolling if the content exceeds the available width
        self.target_list_display.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.target_list_display.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)

        # Allow manual resizing of the columns (on the horizontal header)
        header.setSectionResizeMode(QHeaderView.Interactive)

        # Disable editing of table cells
        self.target_list_display.setEditTriggers(QAbstractItemView.NoEditTriggers)

        # Enable horizontal scrolling by adding the table to a scroll area
        scroll_area = QScrollArea()
        scroll_area.setWidget(self.target_list_display)
        scroll_area.setWidgetResizable(True)  # Ensure that the scroll area resizes with the window
        scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)  # Make horizontal scrollbar always visible

        # Add the scroll area to the layout instead of the table directly
        bottom_section_layout.addWidget(scroll_area)

        # Initially, hide the table
        self.target_list_display.setVisible(False)

        # Connect the selectionChanged signal to the update_target_info function in LogicService
        self.target_list_display.selectionModel().selectionChanged.connect(self.update_target_info)

        target_list_group.setLayout(bottom_section_layout)

        return target_list_group

    
    def on_row_selected(self):
        # Get the selected row's index
        selected_rows = self.parent.target_list_display.selectionModel().selectedRows()
        if selected_rows:
            selected_row = selected_rows[0].row()  # Get the first selected row (you can handle multi-row selection here)
            # Pass the selected row to LogicService
            self.parent.logic_service.update_target_information(selected_row)

    def update_target_info(self):
        # Get the selected row's index
        selected_rows = self.target_list_display.selectionModel().selectedRows()
        if selected_rows:
            selected_row = selected_rows[0].row()  # Get the first selected row (you can handle multi-row selection here)

            # Get the column headers dynamically
            column_headers = [self.target_list_display.horizontalHeaderItem(i).text() for i in range(self.target_list_display.columnCount())]

            # Create a dictionary to hold the target data from the selected row
            target_data = {}

            # Variable to store the OBSERVATION_ID
            observation_id = None

            print("Selected Row:", selected_row)  # Print the selected row index
            print("Column Headers:", column_headers)  # Print the column headers

            for col_index, header in enumerate(column_headers):
                # Get the value from the selected row in each column
                item = self.target_list_display.item(selected_row, col_index)
                value = item.text() if item else ""  # Get text or default to an empty string
                target_data[header] = value  # Add the data to the dictionary

                # Check if the header is 'OBSERVATION_ID' and extract its value
                if header == 'OBSERVATION_ID':
                    observation_id = value  # Store the observation ID
                    print(f"Found OBSERVATION_ID: {observation_id}")  # Print the found OBSERVATION_ID
                    
                # Check if the header is 'Exposure Time' and extract its value
                if header == 'EXPTIME':
                    exposure_time = value  # Store the exposure time
                    print(f"Found Exposure Time: {exposure_time}")  # Print the found exposure time

                # Check if the header is 'Slit Width' and extract its value
                if header == 'SLITWIDTH':
                    slit_width = value  # Store the slit width
                    print(f"Found Slit Width: {slit_width}")  # Print the found slit width
                
                # If the header is 'NAME', store it as the target name
                if header == 'NAME':
                    target_name = value

            # Pass the dictionary of target data to LogicService
            print("Target Data:", target_data)  # Print the full target data for the selected row
            self.parent.logic_service.update_target_information(target_data)

            if observation_id:
                # Store the observation_id in a class variable for later use when the "Go" button is clicked
                self.current_observation_id = observation_id
            if exposure_time:
                self.current_exposure_time = exposure_time
                self.exposure_time_box.setText(exposure_time)  # Update the Exposure Time field
            if slit_width:
                self.current_slit_width = slit_width
                self.slit_width_box.setText(slit_width)  # Update the Slit Width field
            if target_name:
                self.target_name_label.setText(f"Selected Target: {target_name}")
            else:
                self.target_name_label.setText("Selected Target: Not Selected")

            # Enable the "Go" button when a row is selected
            self.go_button.setEnabled(True)  # Enable the "Go" button
            self.go_button.setStyleSheet("""
                QPushButton {
                    background-color: #4CAF50;  /* Green when enabled */
                    color: white;
                    font-weight: bold;
                    padding: 10px;
                    border: none;
                    border-radius: 5px;  /* Optional: Round corners */
                }
                QPushButton:hover {
                    background-color: #388E3C;  /* Darker green when hovered */
                }
                QPushButton:pressed {
                    background-color: #2C6B2F;  /* Even darker green when pressed */
                }
            """)

        else:
            # Disable the "Go" button when no row is selected
            print("No row selected.")  # Print when no row is selected
            self.go_button.setEnabled(False)
            self.go_button.setStyleSheet("""
                QPushButton {
                    background-color: #D3D3D3;  /* Light gray when disabled */
                    color: black;
                    font-weight: bold;
                    padding: 10px;
                    border: none;
                    border-radius: 5px;  /* Optional: Round corners */
                }
                QPushButton:hover {
                    background-color: #D3D3D3;  /* No hover effect when disabled */
                }
                QPushButton:pressed {
                    background-color: #D3D3D3;  /* No pressed effect when disabled */
                }
            """)

    def on_go_button_click(self):
        """Slot to handle 'Go' button click and send the target command."""
        if hasattr(self, 'current_observation_id'):
            observation_id = self.current_observation_id
            print(f"Sending command: seq targetsingle {observation_id}")  # Print the command being sent
            self.send_target_command(observation_id)

            # Disable the button immediately after the user clicks it
            self.go_button.setEnabled(False)
            self.go_button.setStyleSheet("""
                QPushButton {
                    background-color: #D3D3D3;  /* Light gray when disabled */
                    color: black;
                    font-weight: bold;
                    padding: 10px;
                    border: none;
                    border-radius: 5px;  /* Optional: Round corners */
                }
                QPushButton:hover {
                    background-color: #D3D3D3;  /* No hover effect when disabled */
                }
                QPushButton:pressed {
                    background-color: #D3D3D3;  /* No pressed effect when disabled */
                }
            """)

            # Start a QTimer to re-enable the button after 60 seconds
            self.timer = QTimer(self)
            self.timer.setSingleShot(True)  # Ensure the timer only runs once
            self.timer.timeout.connect(self.enable_go_button)
            self.timer.start(60000)  # Timeout after 60 seconds (60000 ms)

        else:
            print("No observation ID available.")
        
    def enable_go_button(self):
        """Method to re-enable the 'Go' button after 60 seconds."""
        print("60 seconds have passed. Re-enabling 'Go' button.")
        
        # Re-enable the button and reset its appearance
        self.go_button.setEnabled(True)
        self.go_button.setStyleSheet("""
            QPushButton {
                background-color: #4CAF50;  /* Green when enabled */
                color: white;
                font-weight: bold;
                padding: 10px;
                border: none;
                border-radius: 5px;  /* Optional: Round corners */
            }
            QPushButton:hover {
                background-color: #388E3C;  /* Darker green when hovered */
            }
            QPushButton:pressed {
                background-color: #2C6B2F;  /* Even darker green when pressed */
            }
        """)
            
    def send_target_command(self, observation_id):
        """ Method to send the command to the SequencerService """
        if observation_id:
            # Build the command string
            command = f"seq targetsingle {observation_id} \n"
            print(f"Sending command to SequencerService: {command}")  # Print the command being sent
            # Call send_command method from SequencerService
            self.parent.send_command(command)
            print(f"Command sent: {command}")  # Print confirmation of command sent
        else:
            print("No OBSERVATION_ID to send the command.")  # Print if no observation ID is found


    # Getter method to access target_list_display from LogicService
    def get_target_list_display(self):
        return self.target_list_display 

    def set_column_widths(self):
        # Set specific column widths (adjust as needed)
        column_widths = [
            150, 150, 150, 250, 250, 250, 250, 150, 150, 250, 150, 150, 150, 150, 150, 150, 150, 150
        ]
        for col, width in enumerate(column_widths):
            self.target_list_display.setColumnWidth(col, width)
            
    def create_second_column_top_half(self):
        """Create the top half of the second column with tabs: 'Planning' and 'Single Target Mode'"""

        # Create a QVBoxLayout to hold everything in the top half
        second_column_top_half_layout = QVBoxLayout()

        # Create a QTabWidget to hold the tabs (Planning and Single Target Mode)
        self.parent.tabs = QTabWidget()

        # Create the two tabs: Planning and Single Target Mode
        self.parent.planning_tab = QWidget()
        self.parent.single_target_tab = QWidget()

        # Add the tabs to the QTabWidget
        self.parent.tabs.addTab(self.parent.planning_tab, "Planning")
        self.parent.tabs.addTab(self.parent.single_target_tab, "Single Target Mode")

        # Set up the layout for the "Planning" tab and add the planning info group
        planning_layout = QVBoxLayout()
        planning_group = self.create_planning_info_group()
        planning_layout.addWidget(planning_group)
        self.parent.planning_tab.setLayout(planning_layout)

        # Set up the layout for the "Single Target Mode" tab
        single_target_layout = QVBoxLayout()
        single_target_label = QLabel("Single Target Mode content goes here.")  # Placeholder for now
        single_target_layout.addWidget(single_target_label)
        self.parent.single_target_tab.setLayout(single_target_layout)

        # Add the QTabWidget to the second column's top half layout
        second_column_top_half_layout.addWidget(self.parent.tabs)

        # Set the layout for the second_column_top_half (containing the tabs)
        second_column_top_half = QWidget()
        second_column_top_half.setLayout(second_column_top_half_layout)

        # Optional: Set maximum size for the group if needed
        second_column_top_half.setMaximumHeight(350)
        second_column_top_half.setMaximumWidth(700)

        return second_column_top_half

    def create_planning_info_group(self):
        planning_group = QGroupBox()
        planning_layout = QHBoxLayout()

        # Create the left and right planning columns
        left_planning_column = self.create_left_planning_column()
        right_planning_column = self.create_right_planning_column()

        # Create a spacer to add space between the columns
        spacer = QSpacerItem(20, 20, QSizePolicy.Expanding, QSizePolicy.Minimum)  # 20px horizontal spacing

        # Add the left column, spacer, and right column to the layout
        planning_layout.addLayout(left_planning_column)
        planning_layout.addItem(spacer)  # This will add space between the two columns
        planning_layout.addLayout(right_planning_column)

        # Set maximum size and size policy for the entire planning group
        planning_group.setLayout(planning_layout)
        planning_group.setMaximumHeight(350)  # Set max height if necessary
        planning_group.setMaximumWidth(700)  # Set max width if necessary
        return planning_group

    def create_left_planning_column(self):
        left_planning_column = QVBoxLayout()
        left_planning_column.setSpacing(10)

        self.parent.start_date_time_edit = QDateTimeEdit()
        self.parent.start_date_time_edit.setDateTime(QDateTime.currentDateTime())
        self.parent.start_date_time_edit.setDisplayFormat("MM/dd/yyyy h:mm AP")

        self.parent.seeing = QLineEdit("1.0")
        self.parent.airmass_limit = QLineEdit("2.0")
        self.parent.target_list_name = QComboBox()

        # Set size policies to ensure these widgets don't grow too large
        self.parent.seeing.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.parent.airmass_limit.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.parent.target_list_name.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        left_planning_column.addWidget(QLabel("Start Date & Time (PST)"))
        left_planning_column.addWidget(self.parent.start_date_time_edit)
        left_planning_column.addWidget(QLabel("Seeing (arcsec)"))
        left_planning_column.addWidget(self.parent.seeing)
        left_planning_column.addWidget(QLabel("Airmass Limit"))
        left_planning_column.addWidget(self.parent.airmass_limit)
        left_planning_column.addWidget(QLabel("Target List"))
        left_planning_column.addWidget(self.parent.target_list_name)

        return left_planning_column

    def create_right_planning_column(self):
        right_planning_column = QVBoxLayout()
        right_planning_column.setSpacing(10)

        self.parent.utc_start_time = QLineEdit("12:00:00 UTC")
        self.parent.twilight_button = QPushButton("Twilight")
        self.parent.twilight_auto_checkbox = QCheckBox("Auto")
        self.parent.fetch_live_button = QPushButton("Fetch Live")
        self.parent.fetch_live_auto_checkbox = QCheckBox("Auto")

        # Set size policies for right column widgets
        self.parent.utc_start_time.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.parent.twilight_button.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.parent.twilight_auto_checkbox.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.parent.fetch_live_button.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.parent.fetch_live_auto_checkbox.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)

        check_x_layout = self.create_check_x_layout()

        right_planning_column.addWidget(QLabel("UTC Start Time"))
        right_planning_column.addWidget(self.parent.utc_start_time)
        right_planning_column.addWidget(self.parent.twilight_button)
        right_planning_column.addWidget(self.parent.twilight_auto_checkbox)
        right_planning_column.addWidget(self.parent.fetch_live_button)
        right_planning_column.addWidget(self.parent.fetch_live_auto_checkbox)
        right_planning_column.addLayout(check_x_layout)

        return right_planning_column

    def create_check_x_layout(self):
        check_x_layout = QHBoxLayout()
        check_x_layout.setSpacing(10)

        # Checkmark button
        self.parent.giant_checkmark_button = QPushButton("\u2713")
        self.parent.giant_checkmark_button.setStyleSheet("""
            QPushButton {
                background-color: green;
                color: white;
                font-size: 80px;
                width: 80px;
                height: 80px;
                text-align: center;
                border: none;
                border-radius: 10px;
            }
            QPushButton:hover {
                background-color: darkgreen;
            }
        """)

        # X button
        self.parent.giant_x_button = QPushButton("\u2717")
        self.parent.giant_x_button.setStyleSheet("""
            QPushButton {
                background-color: red;  /* Red background */
                color: white;  /* White text color */
                font-size: 80px;
                width: 80px;
                height: 80px;
                text-align: center;
                border: none;
                border-radius: 10px;
            }
            QPushButton:hover {
                background-color: darkred;  /* Dark red background on hover */
            }
        """)

        # Add buttons to the layout
        check_x_layout.addWidget(self.parent.giant_checkmark_button)
        check_x_layout.addWidget(self.parent.giant_x_button)

        return check_x_layout


    def create_target_info_group(self):
        # Create a QGroupBox for the target information (with the title "Target Information")
        target_info_group = QGroupBox("Target Information")

        # Create the layout for the group box
        target_info_layout = QVBoxLayout()

        # Create a separate widget to hold the scrollable content (scroll area)
        content_widget = QWidget()

        # Create a QVBoxLayout to hold the content inside the scrollable area
        content_layout = QVBoxLayout()
        content_widget.setLayout(content_layout)

        # Create the "No target selected" label and center it inside the scrollable area
        self.no_target_label = QLabel("No target selected")
        self.no_target_label.setAlignment(Qt.AlignCenter)  # Align the label in the center
        content_layout.addWidget(self.no_target_label)

        # Create the QScrollArea and make the content widget scrollable
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)  # Ensure resizing of the content widget
        scroll_area.setWidget(content_widget)  # Set the content widget as the scrollable area

        # Add the scroll area to the layout of the group box
        target_info_layout.addWidget(scroll_area)

        # Set the layout for the group box
        target_info_group.setLayout(target_info_layout)

        # Return the group box containing the title and scrollable content
        return target_info_group


    def update_target_info_form(self, target_data):
        # Clear the current form before updating
        for i in range(self.target_info_form.rowCount()):
            self.target_info_form.removeRow(0)

        # Add each key-value pair to the form
        for key, value in target_data.items():
            # Create a label and a line edit for each key-value pair
            label = QLabel(key)
            line_edit = QLineEdit(value if value else "N/A")  # Show N/A for empty values
            line_edit.setReadOnly(True)  # Set the line edit to read-only to prevent editing

            # Add the label and line edit to the form layout
            self.target_info_form.addRow(label, line_edit)

        # Optionally, update the widget (forces a refresh)
        self.target_info_form.update()

    def create_control_tab(self, control_tab):
        # Create the main layout for the Control tab
        control_layout = QVBoxLayout()

        # Row 1 and Row 2 (Image Path, Exposure Time, Slit Width) combined
        row_widget = QWidget()
        row_layout = QVBoxLayout(row_widget)
        row_layout.setContentsMargins(0, 0, 0, 0)  # Remove margins
        row_layout.setSpacing(0)  # Remove vertical spacing between widgets

        # Create a new horizontal layout for the target name
        # Create the QLabel with default text
        self.target_name_label = QLabel("Selected Target: Not Selected") 
        self.target_name_label.setAlignment(Qt.AlignCenter) 
        row_layout.addWidget(self.target_name_label)
        row_layout.setAlignment(Qt.AlignLeft)

        # Add label and input field to HBox layout
        row_layout.addWidget(self.target_name_label)



        # Exposure Time and Slit Width Section (Row 2) - Horizontal layout
        row2_layout = QHBoxLayout()
        row2_layout.setContentsMargins(0, 0, 0, 0)  # Remove margins
        row2_layout.setSpacing(5)  # Minimal spacing between widgets in this row

        self.exposure_time_label = QLabel("Exposure Time:")
        self.exposure_time_box = QLineEdit()
        self.exposure_time_box.setPlaceholderText("Enter Exposure Time")

        self.slit_width_label = QLabel("Slit Width:")
        self.slit_width_box = QLineEdit()
        self.slit_width_box.setPlaceholderText("Enter Slit Width")

        # Add Exposure Time and Slit Width widgets to row2_layout
        row2_layout.addWidget(self.exposure_time_label)
        row2_layout.addWidget(self.exposure_time_box)
        row2_layout.addWidget(self.slit_width_label)
        row2_layout.addWidget(self.slit_width_box)

        # Add the Exposure Time and Slit Width layout to the main layout
        row_layout.addLayout(row2_layout)

        # Add the entire row_widget (Row 1 + Row 2) to the control_layout
        control_layout.addWidget(row_widget)

        # Add a thin gray line between rows
        self.add_separator_line(control_layout)

        # --- Add Row 3 layout for "Go" button ---
        row3_widget = QWidget()  # Row 3 - Go Button
        row3_layout = QHBoxLayout(row3_widget)
        row3_layout.setContentsMargins(0, 0, 0, 0)  # Remove margins for Row 3
        row3_layout.setSpacing(10)  # Add spacing between widgets in this row

        # "Go" Button
        self.go_button = QPushButton("Go")
        self.go_button.clicked.connect(self.on_go_button_click)
        self.go_button.setEnabled(False)  # Initially disabled

        # Add the "Go" button to row3_layout
        row3_layout.addWidget(self.go_button)

        # Add Row 3 layout to the control layout
        control_layout.addWidget(row3_widget)

        # Add a thin gray line between rows
        self.add_separator_line(control_layout)

        # --- Add Row 4 layout for Pause, Stop After Current Image, Stop Now Buttons ---
        row4_widget = QWidget()  # Row 4 - Control Buttons
        row4_layout = QHBoxLayout(row4_widget)
        row4_layout.setSpacing(10)
        row4_widget.setContentsMargins(0, 0, 0, 0)  # Remove margins for Row 4

        # Expose Button
        self.expose = QPushButton("Expose")

        # Pause Button
        self.pause_button = QPushButton("Pause")
        self.pause_button.setMaximumWidth(150)

        # Stop Now Button
        self.stop_now_button = QPushButton("Stop Now")
        self.stop_now_button.setMaximumWidth(150)

        # Add buttons to row4_layout
        row4_layout.addWidget(self.expose)
        row4_layout.addWidget(self.pause_button)
        row4_layout.addWidget(self.stop_now_button)

        # Add Row 4 to the main layout
        control_layout.addWidget(row4_widget)

        # Add a thin gray line between rows
        self.add_separator_line(control_layout)

        # --- Add Row 5 layout for Buttons ---
        row5_widget = QWidget()  # Row 5 - Buttons (Binning, Headers, etc.)
        row5_layout = QHBoxLayout(row5_widget)
        row5_layout.setSpacing(10)
        row5_widget.setContentsMargins(0, 0, 0, 0)  # Remove margins for Row 5

        # Binning Button
        self.binning_button = QPushButton("Binning")
        self.binning_button.setMaximumWidth(150)

        # Headers and Display Buttons stacked vertically
        headers_display_layout = QVBoxLayout()
        self.headers_button = QPushButton("Headers")
        self.display_button = QPushButton("Display")
        headers_display_layout.addWidget(self.headers_button)
        headers_display_layout.addWidget(self.display_button)

        # Temp and Lamps Buttons stacked vertically
        temp_lamps_layout = QVBoxLayout()
        self.temp_button = QPushButton("Temp")
        self.lamps_button = QPushButton("Lamps")
        temp_lamps_layout.addWidget(self.temp_button)
        temp_lamps_layout.addWidget(self.lamps_button)

        # Add widgets to row5_layout
        row5_layout.addWidget(self.binning_button)
        row5_layout.addLayout(headers_display_layout)
        row5_layout.addLayout(temp_lamps_layout)

        # Add Row 5 layout to the control layout
        control_layout.addWidget(row5_widget)

        # Set the layout for the control tab
        control_tab.setLayout(control_layout)


    def add_separator_line(self, layout):
        """ Helper method to add a thin light gray line (separator) between rows. """
        separator = QFrame()
        separator.setFrameShape(QFrame.HLine)
        separator.setFrameShadow(QFrame.Sunken)
        separator.setLineWidth(1)  # Thin line
        separator.setStyleSheet("background-color: lightgray;")  # Light gray line
        layout.addWidget(separator)