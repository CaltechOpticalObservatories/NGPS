from PyQt5.QtWidgets import QVBoxLayout, QAbstractItemView,  QHBoxLayout, QTableWidget, QHeaderView, QFormLayout, QListWidget, QListWidgetItem, QScrollArea, QVBoxLayout, QGroupBox, QGroupBox, QHeaderView, QLabel, QRadioButton, QProgressBar, QLineEdit, QTextEdit, QTableWidget, QComboBox, QDateTimeEdit, QTabWidget, QWidget, QPushButton, QCheckBox,QSpacerItem, QSizePolicy
from PyQt5.QtCore import QDateTime, QTimer
from PyQt5.QtGui import QColor, QFont
from instrument_status_service import InstrumentStatusService
from logic_service import LogicService
from PyQt5.QtCore import Qt
from control_tab import ControlTab

class LayoutService:
    def __init__(self, parent):
        self.parent = parent
        self.instrument_status_service = InstrumentStatusService(self.parent)
        self.logic_service = LogicService(self.parent)
        self.target_list_display = None 
        self.target_list_name = QComboBox()

        # Create the control tab instance
        self.control_tab = ControlTab(self.parent)

    def create_layout(self):
        # Main horizontal layout for overall structure
        main_layout = QHBoxLayout()

        # Create a vertical layout to place Column 1 on top of Column 2
        top_layout = QVBoxLayout()

        # First Column (Column 1) should contain top_section_layout and second_column_top_half side by side
        first_column_layout = self.create_first_column()
        top_layout.addLayout(first_column_layout, stretch=3)  # Column 1

        # Second Column (Column 2) should only contain the target_list_group
        second_column_layout = self.create_second_column_for_target_list()
        top_layout.addLayout(second_column_layout, stretch=2)  # Column 2 (only target list)

        # Add the vertical top_layout to the main_layout
        main_layout.addLayout(top_layout, stretch=5)  # Top section (Columns 1 and 2 stacked)

        # Third Column (1/5 width, for tabs, etc.)
        third_column_layout = self.create_third_column()
        main_layout.addLayout(third_column_layout, stretch=1)

        # Set the main layout
        central_widget = QWidget()
        self.parent.setCentralWidget(central_widget)
        central_widget.setLayout(main_layout)

        return main_layout

    def create_first_column(self):
        first_column_layout = QHBoxLayout()  # Use QHBoxLayout to make top_section_layout and second_column_top_half side by side
        first_column_layout.setObjectName("column-left")
        first_column_layout.setSpacing(10)

        # Add top_section_layout (Instrument System Status, Sequencer Mode, Progress & Image Info)
        top_section_layout = self.create_top_section()
        first_column_layout.addLayout(top_section_layout)

        # Add create_second_column_top_half (top part of the second column)
        second_column_top_half = self.create_second_column_top_half()
        first_column_layout.addWidget(second_column_top_half)

        return first_column_layout

    def create_second_column_for_target_list(self):
        second_column_layout = QVBoxLayout()
        second_column_layout.setObjectName("column-right")
        second_column_layout.setSpacing(10)

        # Only add the target list group in the second column
        target_list_group = self.create_target_list_group()
        second_column_layout.addWidget(target_list_group)

        return second_column_layout

    def create_third_column(self):
        third_column_layout = QVBoxLayout()
        third_column_layout.setObjectName("column-sidebar")
        third_column_layout.setSpacing(10)

        # Add widgets to the third column, e.g., tabs, buttons, etc.
        # For simplicity, let's assume it's a placeholder widget:
        sidebar_widget = QWidget()
        third_column_layout.addWidget(sidebar_widget)

        return third_column_layout

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

        # Now, create and set up the layout for the Control tab
        # Create a layout for the Control tab using the ControlTab class
        self.control_tab = ControlTab(self.parent)  # Create the control tab instance
        control_layout = QVBoxLayout()  # You can define a custom layout for the control tab here if needed
        control_layout.addWidget(self.control_tab)  # Add the ControlTab widget to the layout
        self.parent.control_tab.setLayout(control_layout)  # Set the layout for the control tab widget

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

        # Create the QLineEdit widgets
        self.parent.image_name = QLineEdit("NGPS_2024_11_21_Image_1.fits")
        self.parent.image_number = QLineEdit("1")

        # Set the image_name widget to stretch and fill available space
        self.parent.image_name.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        # Set the image_number widget to be smaller
        self.parent.image_number.setFixedWidth(80)  # You can adjust the width as needed

        # Add the QLabel and QLineEdit widgets to the layout
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

        # Optionally set a minimum height or width if desired
        self.parent.message_log.setMinimumHeight(60)
        self.parent.message_log.setMinimumWidth(200)

        # Set up a timer to clear the message log every 10 minutes (600000 milliseconds)
        self.clear_timer = QTimer(self.parent)
        self.clear_timer.timeout.connect(self.clear_message_log)
        self.clear_timer.start(600000)  # 600000 ms = 10 minutes

        return self.parent.message_log

    def clear_message_log(self):
        """ Clears the message log after a certain timeout. """
        self.parent.message_log.clear()

    
    def update_message_log(self, message):
        MAX_LOG_SIZE = 1000  # Max number of characters in the log
        MAX_MESSAGES = 100  # Max number of messages in the log
        """ Update the message log with the new message, maintaining a limit on the size. """
        if self.parent.message_log:
            current_text = self.parent.message_log.toPlainText()

            # Add the new message
            updated_text = current_text + "\n" + message
            
            # Limit the log to the most recent MAX_LOG_SIZE characters
            if len(updated_text) > MAX_LOG_SIZE:
                updated_text = updated_text[-MAX_LOG_SIZE:]

            # Optionally, limit to the most recent MAX_MESSAGES messages
            messages = updated_text.split("\n")
            if len(messages) > MAX_MESSAGES:
                messages = messages[-MAX_MESSAGES:]
            
            updated_text = "\n".join(messages)

            # Update the message log with the new, trimmed text
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

        self.load_target_button.clicked.connect(self.parent.on_login)  # Connect to load CSV functionality
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

        # Enable sorting on column headers
        self.target_list_display.setSortingEnabled(True)

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

            # Variables to store the target values
            observation_id = None
            exposure_time = None
            slit_width = None
            target_name = None
            offset_ra = None
            offset_dec = None

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

                # Check if the header is 'RA' and extract its value
                if header == 'OFFSET_RA':
                    offset_ra = value  # Store the RA
                    print(f"Found OFFSET_RA: {offset_ra}")  # Print the found RA 

                # Check if the header is 'DEC' and extract its value
                if header == 'OFFSET_DEC':
                    offset_dec = value  # Store the DEC
                    print(f"Found OFFSET_DEC: {offset_dec}")  # Print the found DEC 
                
                # If the header is 'NAME', store it as the target name
                if header == 'NAME':
                    target_name = value

            # Pass the dictionary of target data to LogicService
            print("Target Data:", target_data)  # Print the full target data for the selected row
            self.parent.logic_service.update_target_list_table(target_data)

            # Call to set the column widths (adjust them as needed)
            self.set_column_widths()

            if observation_id:
                # Store the observation_id in a class variable for later use when the "Go" button is clicked
                self.parent.current_observation_id = observation_id
                self.parent.current_offset_ra = offset_ra
                self.parent.current_offset_dec = offset_dec
            # if exposure_time:
            #     self.current_exposure_time = exposure_time
            #     self.exposure_time_box.setText(exposure_time)  # Update the Exposure Time field
            # if slit_width:
            #     self.current_slit_width = slit_width
            #     self.slit_width_box.setText(slit_width)  # Update the Slit Width field
            if target_name:
                self.control_tab.target_name_label.setText(f"Selected Target: {target_name}")
            else:
                self.control_tab.target_name_label.setText("Selected Target: Not Selected")

            # Enable the "Go" button when a row is selected
            self.control_tab.go_button.setEnabled(True)  # Enable the "Go" button
            self.control_tab.go_button.setStyleSheet("""
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

            # Now, let's update the corresponding cell in the table for the Exposure Time (or any other field)
            if exposure_time and selected_row is not None:
                # Find the column index for 'EXPTIME'
                exptime_column_index = column_headers.index('EXPTIME') if 'EXPTIME' in column_headers else -1
                if exptime_column_index >= 0:
                    # Update the table cell value for the selected row and 'EXPTIME' column
                    self.target_list_display.item(selected_row, exptime_column_index).setText(exposure_time)

            # You can add similar updates for other fields like SLITWIDTH if needed
            if slit_width and selected_row is not None:
                slitwidth_column_index = column_headers.index('SLITWIDTH') if 'SLITWIDTH' in column_headers else -1
                if slitwidth_column_index >= 0:
                    self.target_list_display.item(selected_row, slitwidth_column_index).setText(slit_width)

        else:
            # Disable the "Go" button when no row is selected
            print("No row selected.")  # Print when no row is selected
            self.control_tab.go_button.setEnabled(False)
            self.control_tab.go_button.setStyleSheet("""
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


    # Getter method to access target_list_display from LogicService
    def get_target_list_display(self):
        return self.target_list_display 

    def set_column_widths(self):
        # Set specific column widths (adjust as needed)
        column_widths = [
            250, 450, 300, 450, 450, 450, 350, 300, 300, 350, 300, 300, 300, 300, 300, 300, 300, 300
        ]

        # Get the number of columns in the table
        column_count = self.target_list_display.columnCount()

        # Ensure we don't exceed the number of available columns
        for col in range(column_count):
            # Use the width from the list, or a default width if the list is too short
            width = column_widths[col] if col < len(column_widths) else 100
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
        self.parent.tabs.addTab(self.parent.single_target_tab, "ETC")

        # Set up the layout for the "Planning" tab and add the planning info group
        planning_layout = QVBoxLayout()
        planning_group = self.create_planning_info_group()
        planning_layout.addWidget(planning_group)
        self.parent.planning_tab.setLayout(planning_layout)

        # Set up the layout for the "Single Target Mode" tab
        single_target_layout = QVBoxLayout()
        single_target_label = QLabel("ETC content goes here.")  # Placeholder for now
        single_target_layout.addWidget(single_target_label)
        self.parent.single_target_tab.setLayout(single_target_layout)

        # Add the QTabWidget to the second column's top half layout
        second_column_top_half_layout.addWidget(self.parent.tabs)

        # Set the layout for the second_column_top_half (containing the tabs)
        second_column_top_half = QWidget()
        second_column_top_half.setLayout(second_column_top_half_layout)

        # Optional: Set maximum size for the group if needed (can be adjusted depending on available space)
        # We might want to make sure this is flexible enough, considering the first column is expanding.
        second_column_top_half.setMaximumHeight(350)  # This can be adjusted based on your design requirements
        second_column_top_half.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Allow vertical resizing if needed

        return second_column_top_half


    def create_planning_info_group(self):
        # Create a group box for planning information
        planning_group = QGroupBox("Planning Information")
        planning_layout = QHBoxLayout()

        # Create the left and right planning columns
        left_planning_column = self.create_left_planning_column()
        right_planning_column = self.create_right_planning_column()

        # Add the columns to the layout with stretch factors to control space allocation
        planning_layout.addLayout(left_planning_column, stretch=2)  # Left column takes more space
        # Add a spacer to create a minimum gap between the left and right columns
        spacer = QSpacerItem(20, 0, QSizePolicy.Fixed, QSizePolicy.Minimum)
        planning_layout.addItem(spacer)
        planning_layout.addLayout(right_planning_column, stretch=1)  # Right column takes less space

        # Set the layout for the entire planning group
        planning_group.setLayout(planning_layout)

        # Optional: Set maximum size and size policy for the entire planning group (optional)
        planning_group.setMaximumHeight(350)  # Max height
        planning_group.setMaximumWidth(700)  # Max width

        return planning_group


    def create_left_planning_column(self):
        # Create the main vertical layout for the left planning column
        left_planning_column = QVBoxLayout()
        left_planning_column.setSpacing(10)  # Space between widgets
        left_planning_column.setContentsMargins(0, 0, 0, 0)  # Optional: Remove margins for better alignment
        self.load_target_lists()  # Call the method to load target lists

        # Create and add widgets to the left column
        self.parent.start_date_time_edit = QDateTimeEdit()
        self.parent.start_date_time_edit.setDateTime(QDateTime.currentDateTime())
        self.parent.start_date_time_edit.setDisplayFormat("MM/dd/yyyy h:mm AP")
        self.parent.start_date_time_edit.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        
        self.parent.seeing = QLineEdit("1.0")
        self.parent.seeing.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        
        self.parent.airmass_limit = QLineEdit("2.0")
        self.parent.airmass_limit.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        
        self.target_list_name.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        # Create horizontal layouts for each pair of label and widget (like buttons and input fields)
        start_date_layout = QVBoxLayout()
        label = QLabel("Start Date & Time (PST)")
        label.setMaximumHeight(40)  # Set the maximum width for the label
        start_date_layout.addWidget(label)
        start_date_layout.addWidget(self.parent.start_date_time_edit)
        start_date_layout.setAlignment(Qt.AlignLeft)

        seeing_layout = QHBoxLayout()
        seeing_layout.addWidget(QLabel("Seeing (arcsec)"))
        seeing_layout.addWidget(self.parent.seeing)
        seeing_layout.setAlignment(Qt.AlignLeft)

        airmass_layout = QHBoxLayout()
        airmass_layout.addWidget(QLabel("Airmass Limit"))
        airmass_layout.addWidget(self.parent.airmass_limit)
        airmass_layout.setAlignment(Qt.AlignLeft)

        target_list_layout = QVBoxLayout()
        target_label = QLabel("Target List")
        target_label.setMaximumHeight(40)  # Set the maximum width for the label
        target_list_layout.addWidget(target_label)
        target_list_layout.addWidget(self.target_list_name)
        target_list_layout.setAlignment(Qt.AlignLeft)
        self.load_target_lists()  # Call the method to load target lists

        # Add these layouts to the main left_planning_column
        left_planning_column.addLayout(start_date_layout)
        left_planning_column.addLayout(seeing_layout)
        left_planning_column.addLayout(airmass_layout)
        left_planning_column.addLayout(target_list_layout)

        # Optionally, limit the overall width of the left column
        left_planning_column.setContentsMargins(0, 0, 0, 0)  # Remove margins for better control
        left_planning_column.setSpacing(10)  # Space between rows

        return left_planning_column
    
    def load_target_lists(self, target_lists=None):
        """Populate the ComboBox with target lists passed as a parameter."""
        try:
            # If no list is passed, attempt to load target lists from the database or another source
            if target_lists is None:
                target_lists = self.logic_service.load_mysql_and_fetch_target_sets("config/db_config.ini")
                
                # Ensure target_lists is iterable (like a list or tuple)
                if not isinstance(target_lists, (list, tuple)):
                    print("Error: Fetched data is not a valid iterable (list or tuple).")
                    target_lists = []  # Set to empty list if not valid

        except Exception as e:
            # Handle any exception that occurs during fetching
            print(f"Error fetching target lists: {e}")
            target_lists = []  # Set to empty list if an error occurs

        # If target_lists is empty, we can show a fallback message or an empty list
        if not target_lists:
            target_lists = ["No Target Lists Available"]  # Fallback message or an empty list

        # Ensure the ComboBox is cleared before populating it
        if isinstance(self.target_list_name, QComboBox):
            self.target_list_name.clear()

            # Add the fetched or fallback target lists to the ComboBox
            for set_name in target_lists:
                self.target_list_name.addItem(set_name)

            # Set the first item as the default selection (if available)
            if target_lists:
                self.target_list_name.setCurrentIndex(0)  # Set the first item as default

            # Connect the signal for user selection change
            # self.target_list_name.currentIndexChanged.connect(self.on_target_set_changed)


    def on_target_set_changed(self):
        """Handle the target set change in the ComboBox."""
        # Get the selected SET_NAME (not the entire list of target sets)
        selected_set_name = self.target_list_name.currentText()
        
        print(f"Selected SET_NAME: {selected_set_name}")
        self.logic_service.update_target_list_table(self.logic_service.all_targets, self.logic_service.target_list_set, selected_set_name)


    def create_right_planning_column(self):
        right_planning_column = QVBoxLayout()
        right_planning_column.setSpacing(10)
        right_planning_column.setContentsMargins(0, 0, 0, 0)  # Optional: Remove margins for better alignment

        # Create and add widgets to the right column
        self.parent.utc_start_time = QLineEdit("12:00:00 UTC")
        self.parent.utc_start_time.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Ensure this expands correctly

        self.parent.twilight_button = QPushButton("Twilight")
        self.parent.twilight_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Same width for both buttons

        self.parent.twilight_auto_checkbox = QCheckBox("Auto")

        self.parent.fetch_live_button = QPushButton("Fetch Live")
        self.parent.fetch_live_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Same width for both buttons

        self.parent.fetch_live_auto_checkbox = QCheckBox("Auto")

        # Create horizontal layouts for Twilight and Fetch Live buttons + checkboxes
        twilight_layout = QHBoxLayout()
        twilight_layout.addWidget(self.parent.twilight_button)
        twilight_layout.addWidget(self.parent.twilight_auto_checkbox)
        twilight_layout.setAlignment(Qt.AlignLeft)  # Align Twilight button and checkbox to the left

        fetch_live_layout = QHBoxLayout()
        fetch_live_layout.addWidget(self.parent.fetch_live_button)
        fetch_live_layout.addWidget(self.parent.fetch_live_auto_checkbox)
        fetch_live_layout.setAlignment(Qt.AlignLeft)  # Align Fetch Live button and checkbox to the left

        # Add the widgets to the right column layout
        right_planning_column.addWidget(QLabel("UTC Start Time"))
        right_planning_column.addWidget(self.parent.utc_start_time)
        right_planning_column.addLayout(twilight_layout)  # Twilight button and checkbox side by side
        right_planning_column.addLayout(fetch_live_layout)  # Fetch Live button and checkbox side by side

        # If there's an additional layout like checkboxes, add it
        check_x_layout = self.create_check_x_layout()  # Assuming this method exists
        right_planning_column.addLayout(check_x_layout)

        # Add stretch for proportional resizing (optional)
        right_planning_column.setStretch(0, 1)

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
                font-size: 30px;
                width: 30px;
                height: 30px;
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
                font-size: 30px;
                width: 30px;
                height: 30px;
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