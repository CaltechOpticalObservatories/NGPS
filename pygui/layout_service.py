from PyQt5.QtWidgets import QVBoxLayout, QHBoxLayout, QTableWidget, QHeaderView, QFormLayout, QListWidget, QListWidgetItem, QScrollArea, QVBoxLayout, QGroupBox, QGroupBox, QHeaderView, QLabel, QRadioButton, QProgressBar, QLineEdit, QTextEdit, QTableWidget, QComboBox, QDateTimeEdit, QTabWidget, QWidget, QPushButton, QCheckBox,QSpacerItem, QSizePolicy
from PyQt5.QtCore import QDateTime
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

        # Planning Info
        planning_group = self.create_planning_info_group()
        second_column_layout.addWidget(planning_group)

        # Target Information
        target_info_group = self.create_target_info_group()
        second_column_layout.addWidget(target_info_group)

        return second_column_layout

    def create_third_column(self):
        third_column_layout = QVBoxLayout()

        self.parent.tabs = QTabWidget()
        self.parent.control_tab = QWidget()
        self.parent.status_tab = QWidget()
        self.parent.engineering_tab = QWidget()

        self.parent.tabs.addTab(self.parent.control_tab, "Control")
        self.parent.tabs.addTab(self.parent.status_tab, "Status")
        self.parent.tabs.addTab(self.parent.engineering_tab, "Engineering")

        # Set minimum width and height for the QTabWidget
        self.parent.tabs.setMinimumWidth(300)  # Set your desired minimum width
        self.parent.tabs.setMinimumHeight(200)  # Set your desired minimum height

        third_column_layout.addWidget(self.parent.tabs)

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
        self.parent.message_log = QTextEdit()
        
        # Set size policies to allow the widget to stretch and grow proportionally
        self.parent.message_log.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        # Optionally set a minimum height or width if desired (not fixed size)
        self.parent.message_log.setMinimumHeight(60)
        self.parent.message_log.setMinimumWidth(200)  # Set a reasonable minimum width

        return self.parent.message_log

    def create_target_list_group(self):
        target_list_group = QGroupBox("Target List")
        bottom_section_layout = QVBoxLayout()
        bottom_section_layout.setSpacing(5)

        # Create the button to load the target list
        self.load_target_button = QPushButton("Please login or load your target list to start")
        # Set the button color to #FFCC40 (light yellow)
        self.load_target_button.setStyleSheet("""
            QPushButton {
                background-color: #FFCC40;  /* Normal background color */
                border: none;
                color: black;
                font-weight: bold;
                padding: 10px;
            }
            QPushButton:hover {
                background-color: #FF9900;  /* Hover background color */
            }
            QPushButton:pressed {
                background-color: #FF6600;  /* Pressed background color */
            }
        """)

        # Center the button by using a QHBoxLayout
        button_layout = QHBoxLayout()
        button_layout.addWidget(self.load_target_button)
        button_layout.setAlignment(self.load_target_button, Qt.AlignCenter)  # Center the button

        self.load_target_button.clicked.connect(self.parent.load_csv_file)  # Connect to load CSV functionality
        bottom_section_layout.addWidget(self.load_target_button)

        # Create the QTableWidget for the target list (but keep it hidden initially)
        self.target_list_display = QTableWidget()
        self.target_list_display.setRowCount(0)  # Set to 0 initially
        self.target_list_display.setColumnCount(14)  # Set the required column count

        # Define the column headers
        column_headers = [
            "Name", "RA", "Dec", "EXPTime Request", "Exposure Time (s)", 
            "SlitWidth Request", "Slit Width (arcsec)", "Airmass", 
            "OTMSNR", "Observer's Priority", "CCD Mode", "Bin Spectral (int)", 
            "Bin Spatial (int)", "Slit Angle Request (deg)", "Airmass limit", 
            "Point Mode", "Not Before", "Comment"
        ]
        
        # Set the header labels
        self.target_list_display.setHorizontalHeaderLabels(column_headers)

        # Remove the bold font from headers
        header = self.target_list_display.horizontalHeader()
        header.setFont(QFont("Arial", 10, QFont.Normal))  # Set font to normal (non-bold)

        # Set specific column widths (adjust as needed)
        self.set_column_widths()

        # Enable horizontal scrolling if the content exceeds the available width
        self.target_list_display.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.target_list_display.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)

        # Allow manual resizing of the columns (on the horizontal header)
        header.setSectionResizeMode(QHeaderView.Interactive)

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

            # Create a dictionary to hold the target data from the selected row
            target_data = {
                "Name": self.target_list_display.item(selected_row, 0).text() if self.target_list_display.item(selected_row, 0) else "",
                "RA": self.target_list_display.item(selected_row, 1).text() if self.target_list_display.item(selected_row, 1) else "",
                "Declination": self.target_list_display.item(selected_row, 2).text() if self.target_list_display.item(selected_row, 2) else "",
                "EXPTime Request": self.target_list_display.item(selected_row, 3).text() if self.target_list_display.item(selected_row, 3) else "",
                "Exposure Time (s)": self.target_list_display.item(selected_row, 4).text() if self.target_list_display.item(selected_row, 4) else "",
                "SlitWidth Request": self.target_list_display.item(selected_row, 5).text() if self.target_list_display.item(selected_row, 5) else "",
                "Slit Width (arcsec)": self.target_list_display.item(selected_row, 6).text() if self.target_list_display.item(selected_row, 6) else "",
                "Airmass": self.target_list_display.item(selected_row, 7).text() if self.target_list_display.item(selected_row, 7) else "",
                "OTMSNR": self.target_list_display.item(selected_row, 8).text() if self.target_list_display.item(selected_row, 8) else "",
                "Observer's Priority": self.target_list_display.item(selected_row, 9).text() if self.target_list_display.item(selected_row, 9) else "",
                "CCD Mode": self.target_list_display.item(selected_row, 10).text() if self.target_list_display.item(selected_row, 10) else "",
                "Bin Spectral (int)": self.target_list_display.item(selected_row, 11).text() if self.target_list_display.item(selected_row, 11) else "",
                "Bin Spatial (int)": self.target_list_display.item(selected_row, 12).text() if self.target_list_display.item(selected_row, 12) else "",
                "Slit Angle Request (deg)": self.target_list_display.item(selected_row, 13).text() if self.target_list_display.item(selected_row, 13) else "",
                "Airmass limit": self.target_list_display.item(selected_row, 14).text() if self.target_list_display.item(selected_row, 14) else "",
                "Point Mode": self.target_list_display.item(selected_row, 15).text() if self.target_list_display.item(selected_row, 15) else "",
                "Not Before": self.target_list_display.item(selected_row, 16).text() if self.target_list_display.item(selected_row, 16) else "",
                "Comment": self.target_list_display.item(selected_row, 17).text() if self.target_list_display.item(selected_row, 17) else ""
            }

            # Pass the dictionary of target data to LogicService
            self.parent.logic_service.update_target_information(target_data)

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
            

    def create_planning_info_group(self):
        planning_group = QGroupBox("Planning Info")
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
        self.parent.giant_checkmark_button = QPushButton("✔️")
        self.parent.giant_checkmark_button.setStyleSheet("""
            QPushButton {
                background-color: green;
                color: white;
                font-size: 50px;
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
        self.parent.giant_x_button = QPushButton("❌")
        self.parent.giant_x_button.setStyleSheet("""
            QPushButton {
                background-color: red;
                color: white;
                font-size: 50px;
                width: 80px;
                height: 80px;
                text-align: center;
                border: none;
                border-radius: 10px;
            }
            QPushButton:hover {
                background-color: darkred;
            }
        """)

        # Add buttons to the layout
        check_x_layout.addWidget(self.parent.giant_checkmark_button)
        check_x_layout.addWidget(self.parent.giant_x_button)

        return check_x_layout

    def create_target_info_group(self):
        # Create a group box for target information
        target_info_group = QGroupBox("Target Information")

        # Create a vertical layout to hold the QFormLayout
        target_info_layout = QVBoxLayout()

        # Create the QFormLayout to display key-value pairs
        self.target_info_form = QFormLayout()

        # Add the form layout to the parent layout
        target_info_layout.addLayout(self.target_info_form)

        # Create the label that will show "No target selected" in the center
        self.no_target_label = QLabel("No target selected")
        self.no_target_label.setAlignment(Qt.AlignCenter)  # Align the label to the center

        # Add the label to the layout above the form
        target_info_layout.addWidget(self.no_target_label)

        # Create a QScrollArea to make the target info area scrollable
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)  # This allows the widget to resize with the scroll area
        scroll_area.setWidget(target_info_group)  # Set the group box as the widget inside the scroll area

        # Set the layout for the group box (now inside the scroll area)
        target_info_group.setLayout(target_info_layout)

        # Return the scroll area (not the group box directly)
        return scroll_area

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
