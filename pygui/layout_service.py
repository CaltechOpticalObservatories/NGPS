from PyQt5.QtWidgets import QVBoxLayout, QHBoxLayout, QGroupBox, QLabel, QRadioButton, QProgressBar, QLineEdit, QTextEdit, QTableWidget, QComboBox, QDateTimeEdit, QTabWidget, QWidget, QPushButton, QCheckBox
from PyQt5.QtCore import QDateTime
from PyQt5.QtGui import QColor
from instrument_status_service import InstrumentStatusService
from PyQt5.QtCore import Qt  # Import Qt to access alignment constants

class LayoutService:
    def __init__(self, parent):
        self.parent = parent
        self.instrument_status_service = InstrumentStatusService(self.parent)  # Initialize the InstrumentStatusService

    def create_layout(self):
        main_layout = QHBoxLayout()

        # First Column (3/5 width)
        first_column_layout = QVBoxLayout()
        first_column_layout.setObjectName("column-left")

        # Top Section - Instrument System Status, Sequencer Mode, and Progress & Image Info
        top_section_layout = QHBoxLayout()  # Horizontal layout for Instrument System Status + Progress & Image Info

        # Left side: Instrument System Status and Sequencer Mode
        left_top_layout = QVBoxLayout()

        # Instrument System Status
        system_status_group = QGroupBox("Instrument System Status")
        system_status_layout = QVBoxLayout()

        # Initialize the instrument status label and status color rectangles
        self.parent.instrument_status_label = QLabel("System Status:")

        # Create the layout for displaying the status list
        status_list_layout = QVBoxLayout()

        # Create the status map (with color and text)
        status_map = {
            "stopped": QColor(169, 169, 169),  # Grey
            "idle": QColor(255, 255, 0),       # Yellow
            "paused": QColor(255, 165, 0),     # Orange
            "exposing": QColor(0, 255, 0),     # Green
        }

        # Loop over the statuses and add each one to the layout
        for status, color in status_map.items():
            # Create a horizontal layout for each status
            status_layout = QHBoxLayout()

            # Create the color rectangle for the current status
            status_color_rect = QWidget()
            status_color_rect.setFixedSize(20, 20)  # Size of the rectangle
            status_color_rect.setStyleSheet(f"background-color: {color.name()};")

            # Create the label for the current status
            status_label = QLabel(status.capitalize())

            # Add the color rectangle and the label to the horizontal layout
            status_layout.addWidget(status_color_rect)
            status_layout.addWidget(status_label)

            # Add the horizontal layout to the main list layout
            status_list_layout.addLayout(status_layout)

        # Set the main layout for the system status group
        system_status_group.setLayout(status_list_layout)

        # Add the system status label to the top
        status_list_layout.insertWidget(0, self.parent.instrument_status_label)

        left_top_layout.addWidget(system_status_group)

        # Sequencer Mode
        sequencer_mode_group = QGroupBox("Sequencer Mode")
        sequencer_mode_layout = QVBoxLayout()
        self.parent.sequencer_mode_single = QRadioButton("Single")
        self.parent.sequencer_mode_all = QRadioButton("All")
        sequencer_mode_layout.addWidget(self.parent.sequencer_mode_single)
        sequencer_mode_layout.addWidget(self.parent.sequencer_mode_all)
        sequencer_mode_group.setLayout(sequencer_mode_layout)

        left_top_layout.addWidget(sequencer_mode_group)

        # Right side: Progress & Image Info
        right_top_layout = QVBoxLayout()

        progress_and_image_group = QGroupBox("Progress and Image Info")
        progress_and_image_layout = QVBoxLayout()

        # Horizontal Layout for Overhead and Exposure Progress Bars
        progress_layout = QHBoxLayout()
        self.parent.exposure_progress = QProgressBar()
        self.parent.exposure_progress.setRange(0, 100)
        self.parent.overhead_progress = QProgressBar()
        self.parent.overhead_progress.setRange(0, 100)
        progress_layout.addWidget(QLabel("Overhead Progress"))
        progress_layout.addWidget(self.parent.overhead_progress)
        progress_layout.addWidget(QLabel("Exposure Progress"))
        progress_layout.addWidget(self.parent.exposure_progress)

        # Horizontal Layout for Image Name and Image Number
        image_info_layout = QHBoxLayout()
        self.parent.image_name = QLineEdit("Image001")
        self.parent.image_number = QLineEdit("1")
        image_info_layout.addWidget(QLabel("Image Name:"))
        image_info_layout.addWidget(self.parent.image_name)
        image_info_layout.addWidget(QLabel("Image Number:"))
        image_info_layout.addWidget(self.parent.image_number)

        # Messages Layout (Text Area)
        self.parent.message_log = QTextEdit()
        self.parent.message_log.setFixedHeight(60)  # Enough space for 3 lines of messages
        self.parent.message_log.setFixedWidth(350)  # Adjust width as per need

        # Add all elements to progress_and_image_layout
        progress_and_image_layout.addLayout(progress_layout)
        progress_and_image_layout.addLayout(image_info_layout)
        progress_and_image_layout.addWidget(QLabel("Log Messages:"))
        progress_and_image_layout.addWidget(self.parent.message_log)

        progress_and_image_group.setLayout(progress_and_image_layout)
        right_top_layout.addWidget(progress_and_image_group)

        # Combine left and right top sections in horizontal layout
        top_section_layout.addLayout(left_top_layout)
        top_section_layout.addLayout(right_top_layout)

        # Add top_section_layout to the first column layout with stretch factor
        first_column_layout.addLayout(top_section_layout, stretch=1)

        # Bottom Section - Target List
        target_list_group = QGroupBox("Target List")
        bottom_section_layout = QVBoxLayout()
        self.parent.target_list_display = QTableWidget()
        self.parent.target_list_display.setRowCount(10)
        self.parent.target_list_display.setColumnCount(3)
        self.parent.target_list_display.setHorizontalHeaderLabels(["Target Name", "RA", "Dec"])

        bottom_section_layout.addWidget(self.parent.target_list_display)
        target_list_group.setLayout(bottom_section_layout)
        
        # Add Target List section with stretch factor
        first_column_layout.addWidget(target_list_group, stretch=2)

        # Add the first column layout to the main layout
        main_layout.addLayout(first_column_layout)

        # Second Column (1/5 width) - Planning Info and Target Info (vertical split)
        second_column_layout = QVBoxLayout()
        second_column_layout.setObjectName("column-right")

        # Planning Info (split into two equal columns)
        planning_group = QGroupBox("Planning Info")
        planning_layout = QHBoxLayout()  # Horizontal layout for splitting into two columns

        # Right Column (1/2) - UTC Start Time, Twilight Button, Fetch Live Button, Giant Checkmark & X
        right_planning_column = QVBoxLayout()

        # Add QLabel for UTC Start Time
        right_planning_column.addWidget(QLabel("UTC Start Time"))

        # UTC Start Time input
        self.parent.utc_start_time = QLineEdit("12:00:00 UTC")  # Example default UTC Start Time

        # Twilight button with checkbox for auto
        self.parent.twilight_button = QPushButton("Twilight")
        self.parent.twilight_auto_checkbox = QCheckBox("Auto")

        # Fetch Live button with checkbox for auto
        self.parent.fetch_live_button = QPushButton("Fetch Live")
        self.parent.fetch_live_auto_checkbox = QCheckBox("Auto")

        # Giant Checkmark & Giant X - in a horizontal layout
        check_x_layout = QHBoxLayout()

        # Giant green checkmark button (square box with checkmark)
        self.parent.giant_checkmark_button = QPushButton("✔")
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
        """)

        # Giant red X button (square box with X)
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
        """)

        # Add the buttons to the horizontal layout
        check_x_layout.addWidget(self.parent.giant_checkmark_button)
        check_x_layout.addWidget(self.parent.giant_x_button)

        # Add the rest of the right column widgets
        right_planning_column.addWidget(self.parent.utc_start_time)
        right_planning_column.addWidget(self.parent.twilight_button)
        right_planning_column.addWidget(self.parent.twilight_auto_checkbox)
        right_planning_column.addWidget(self.parent.fetch_live_button)
        right_planning_column.addWidget(self.parent.fetch_live_auto_checkbox)
        right_planning_column.addLayout(check_x_layout)  # Add the horizontal layout containing the checkmark and X

        # Left Column (1/2) - Start Date & Time, Seeing, Airmass Limit, Target List
        left_planning_column = QVBoxLayout()

        # Left column widgets (for planning info)
        self.parent.start_date_time_edit = QDateTimeEdit()
        self.parent.start_date_time_edit.setDateTime(QDateTime.currentDateTime())  # Default to current time
        self.parent.start_date_time_edit.setDisplayFormat("MM/dd/yyyy h:mm AP")  # 12-hour format with AM/PM

        self.parent.seeing = QLineEdit("1.0")
        self.parent.airmass_limit = QLineEdit("2.0")
        self.parent.target_list_name = QComboBox()

        # Add left column widgets
        left_planning_column.addWidget(QLabel("Start Date & Time (PST)"))
        left_planning_column.addWidget(self.parent.start_date_time_edit)
        left_planning_column.addWidget(QLabel("Seeing (arcsec)"))
        left_planning_column.addWidget(self.parent.seeing)
        left_planning_column.addWidget(QLabel("Airmass Limit"))
        left_planning_column.addWidget(self.parent.airmass_limit)
        left_planning_column.addWidget(QLabel("Target List"))
        left_planning_column.addWidget(self.parent.target_list_name)

        # Add left and right planning columns into a horizontal layout
        planning_layout = QHBoxLayout()  # New horizontal layout for the two columns

        # Add the left and right columns into the horizontal layout
        planning_layout.addLayout(left_planning_column)
        planning_layout.addLayout(right_planning_column)

        # Set the layout for the Planning Info group
        planning_group.setLayout(planning_layout)


        # Add planning group (1/3 of the second column height)
        second_column_layout.addWidget(planning_group, stretch=1)

        # Target Information (2/3 height)
        target_info_group = QGroupBox("Target Information")
        target_info_layout = QVBoxLayout()

        self.parent.target_info_details = QTextEdit()
        target_info_layout.addWidget(self.parent.target_info_details)

        target_info_group.setLayout(target_info_layout)

        # Add target information group (2/3 of the second column height)
        second_column_layout.addWidget(target_info_group, stretch=2)

        # Add the second column layout to the main layout
        main_layout.addLayout(second_column_layout)

        # Third Column (1/5 width) - Tabs
        third_column_layout = QVBoxLayout()
        self.parent.tabs = QTabWidget()
        self.parent.control_tab = QWidget()
        self.parent.status_tab = QWidget()
        self.parent.engineering_tab = QWidget()

        self.parent.tabs.addTab(self.parent.control_tab, "Control")
        self.parent.tabs.addTab(self.parent.status_tab, "Status")
        self.parent.tabs.addTab(self.parent.engineering_tab, "Engineering")

        third_column_layout.addWidget(self.parent.tabs)

        main_layout.addLayout(third_column_layout)

        # Set the main layout
        central_widget = QWidget()
        self.parent.setCentralWidget(central_widget)
        central_widget.setLayout(main_layout)

        return main_layout
