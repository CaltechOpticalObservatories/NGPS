from PyQt5.QtWidgets import QVBoxLayout, QAbstractItemView, QFrame, QDialog, QFileDialog, QDialogButtonBox, QTableWidgetItem,  QInputDialog, QHBoxLayout, QGridLayout, QTableWidget, QHeaderView, QFormLayout, QListWidget, QListWidgetItem, QScrollArea, QVBoxLayout, QGroupBox, QGroupBox, QHeaderView, QLabel, QRadioButton, QProgressBar, QLineEdit, QTextEdit, QTableWidget, QComboBox, QDateTimeEdit, QTabWidget, QWidget, QPushButton, QCheckBox,QSpacerItem, QSizePolicy
from PyQt5.QtCore import QDateTime, QTimer
from PyQt5.QtGui import QColor, QFont
from instrument_status_service import InstrumentStatusService
from logic_service import LogicService
from PyQt5.QtCore import Qt
from control_tab import ControlTab
from instrument_status_tab import InstrumentStatusTab
import re
import subprocess

class LayoutService:
    def __init__(self, parent):
        self.parent = parent
        self.instrument_status_service = InstrumentStatusService(self.parent)
        self.logic_service = LogicService(self.parent)
        self.target_list_display = None 
        self.target_list_name = QComboBox()
        self.add_row_button = QPushButton()
        self.startup_shutdown_button = QPushButton()

        # Create the control tab instance
        self.control_tab = ControlTab(self.parent)
        
        # Create the instrument status tab instance
        self.instrument_status_tab = InstrumentStatusTab(self.parent)
        
    def get_screen_size_ratio(self):
        # Get the user's screen size
        screen = self.parent.screen()
        screen_size = screen.geometry()
        
        # Use the screen width and height to calculate dynamic ratio (you can adjust these ratios)
        screen_width = screen_size.width()
        screen_height = screen_size.height()

        # Define dynamic ratio logic based on the screen size
        # For example, adjust the layout ratio based on screen size
        if screen_width > 2000:
            # For large screens, give more space to the main layout
            ratio = (5, 2, 1)
        elif screen_width > 1500:
            # For medium-sized screens, keep the ratio balanced
            ratio = (4, 3, 1)
        else:
            # For smaller screens, prioritize the left column
            ratio = (3, 2, 1)
        
        return ratio

    def create_layout(self):
        # Get the layout ratio based on the screen size
        layout_ratio = self.get_screen_size_ratio()

        # Main horizontal layout for overall structure
        main_layout = QHBoxLayout()

        # Create a vertical layout to place Column 1 on top of Column 2
        top_layout = QVBoxLayout()

        # First Column (Column 1) should contain top_section_layout and second_column_top_half side by side
        first_column_layout = self.create_first_column()
        top_layout.addLayout(first_column_layout, stretch=layout_ratio[0])  # Column 1

        # Second Column (Column 2) should only contain the target_list_group
        second_column_layout = self.create_second_column_for_target_list()
        top_layout.addLayout(second_column_layout, stretch=layout_ratio[1])  # Column 2 (only target list)

        # Add the vertical top_layout to the main_layout
        main_layout.addLayout(top_layout, stretch=layout_ratio[0] + layout_ratio[1])  # Top section (Columns 1 and 2 stacked)

        # Third Column (1/5 width, for tabs, etc.)
        third_column_layout = self.create_third_column()
        main_layout.addLayout(third_column_layout, stretch=layout_ratio[2])

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

        self.status_tab = InstrumentStatusTab(self.parent)  # Create the control tab instance
        status_layout = QVBoxLayout()  # You can define a custom layout for the control tab here if needed
        status_layout.addWidget(self.status_tab)  # Add the ControlTab widget to the layout
        self.parent.status_tab.setLayout(status_layout)  # Set the layout for the control tab widget
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
        system_status_layout.setSpacing(3)  # Reduced space between items in the layout
        system_status_layout.setContentsMargins(5, 5, 5, 5)  # Reduced margins around the layout

        # Create a mapping for status colors
        status_map = {
            "stopped": QColor(169, 169, 169),  # Grey
            "idle": QColor(255, 255, 0),       # Yellow
            "paused": QColor(255, 165, 0),     # Orange
            "exposing": QColor(0, 255, 0),     # Green
        }

        # Create a dictionary to hold the status widgets, which we will enable/disable
        self.status_widgets = {}

        # Create status widgets and add them to the layout
        for status, color in status_map.items():
            # Create a QWidget to contain the status layout
            status_widget = QWidget()

            # Create a layout for the status (color + label)
            status_layout = QHBoxLayout()
            status_layout.setSpacing(10)  # Reduced space between color and label
            status_layout.setContentsMargins(0, 0, 0, 0)  # Remove margins for the status layout

            # Color indicator
            status_color_rect = QWidget()
            status_color_rect.setFixedSize(16, 16)  # Smaller color indicator
            status_color_rect.setStyleSheet(f"background-color: {color.name()};")

            # Label showing the status
            status_label = QLabel(status.capitalize())
            status_label.setMargin(0)  # Remove extra margin around the label

            # Layout for each status (color + label)
            status_layout.addWidget(status_color_rect)
            status_layout.addWidget(status_label)

            # Set the layout for the status widget
            status_widget.setLayout(status_layout)

            # Add the status widget to the main layout
            system_status_layout.addWidget(status_widget)

            # Store status widgets in a dictionary for later use
            self.status_widgets[status] = {
                'widget': status_widget,
                'color_rect': status_color_rect,
                'label': status_label,
                'color': color
            }

        # Create the Startup/Shutdown button
        self.startup_shutdown_button = QPushButton("Startup")
        self.startup_shutdown_button.setStyleSheet("""
            QPushButton {
                background-color: #4CAF50;  /* Green for startup */
                border: none;
                color: white;
                font-weight: bold;
                padding: 5px 10px;  /* Reduced padding */
            }
            QPushButton:hover {
                background-color: #388E3C;
            }
            QPushButton:pressed {
                background-color: #2C6B2F;
            }
        """)

        # Button click handler: toggle between Startup and Shutdown
        self.startup_shutdown_button.clicked.connect(self.toggle_startup_shutdown)

        # Add the button to the layout
        system_status_layout.addWidget(self.startup_shutdown_button)

        system_status_group.setLayout(system_status_layout)

        # Set maximum width and height for the system status group
        system_status_group.setMaximumWidth(300)  # Maximum width
        system_status_group.setMaximumHeight(250)  # Maximum height

        # Ensure only the 'stopped' status is fully active by default
        self.update_system_status("stopped")

        return system_status_group

    def toggle_startup_shutdown(self):
        # Get the current button text and toggle
        current_text = self.startup_shutdown_button.text()

        if current_text == "Startup":
            # Change the button to Shutdown (black)
            self.startup_shutdown_button.setText("Shutdown")
            self.startup_shutdown_button.setStyleSheet("""
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
            print("Startup button clicked!")
            command = f"startup\n"
            self.parent.send_command(command)
            self.update_system_status("idle")  # Set to 'idle' when starting up
        else:
            # Change the button back to Startup (green)
            self.startup_shutdown_button.setText("Startup")
            self.startup_shutdown_button.setStyleSheet("""
                QPushButton {
                    background-color: #4CAF50;  /* Green for startup */
                    border: none;
                    color: white;
                    font-weight: bold;
                    padding: 5px 10px;  /* Reduced padding */
                }
                QPushButton:hover {
                    background-color: #388E3C;
                }
                QPushButton:pressed {
                    background-color: #2C6B2F;
                }
            """)
            print("Shutdown button clicked!")
            command = f"shutdown\n"
            self.parent.send_command(command) 
            self.update_system_status("stopped")  # Set to 'stopped' when shutting down

    def update_system_status(self, status):
        # Iterate through all status widgets and "disable" them
        for status_key, status_data in self.status_widgets.items():
            widget = status_data['widget']
            color_rect = status_data['color_rect']
            label = status_data['label']
            original_color = status_data['color']

            if status_key != status:
                # Disable the status widget (make it gray)
                color_rect.setStyleSheet(f"background-color: {QColor(169, 169, 169).name()};")  # Grey color
                label.setStyleSheet("color: #A9A9A9;")  # Gray text for disabled status
            else:
                # Enable the current status (show its original color)
                color_rect.setStyleSheet(f"background-color: {original_color.name()};")
                label.setStyleSheet("color: white;")  # Black text for active status


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
        self.parent.exposure_progress.setValue(0)
        self.parent.exposure_progress.setMaximumWidth(200)  # Max width for progress bar

        self.parent.overhead_progress = QProgressBar()
        self.parent.overhead_progress.setValue(0)
        self.parent.overhead_progress.setRange(0, 100)
        self.parent.overhead_progress.setMaximumWidth(220)  # Max width for progress bar

        progress_layout.addWidget(QLabel("Readout Progress"))
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
        image_info_layout.addWidget(QLabel("Image Dir:"))
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
        target_list_group = QGroupBox()
        bottom_section_layout = QVBoxLayout()
        bottom_section_layout.setSpacing(5)

        # Create a horizontal layout for the label and the (+) button
        header_layout = QHBoxLayout()
        header_layout.setSpacing(10)  # Set the space between the label and the button

        # Create the label with the "Target List" text
        target_list_label = QLabel("Target List")
        header_layout.addWidget(target_list_label)

        # Create the (+) button to add a new row (small button)
        self.add_row_button = QPushButton("+")
        self.add_row_button.setToolTip("Add a new row")
        self.add_row_button.setFixedSize(25, 25)  # Make the button small
        self.add_row_button.setStyleSheet("""
            QPushButton {
                background-color: #4CAF50;
                border: none;
                color: white;
                font-weight: bold;
                font-size: 18px;
                border-radius: 12px;
            }
            QPushButton:hover {
                background-color: #388E3C;
            }
            QPushButton:pressed {
                background-color: #2C6B2F;
            }
            QPushButton:disabled {
                background-color: #D3D3D3;  /* Grey background when disabled */
                color: #A9A9A9;  /* Grey text color when disabled */
            }
        """)
        self.add_row_button.clicked.connect(self.add_new_row)  # Connect to function to add a new row
        self.add_row_button.setEnabled(False)
        # Add the (+) button next to the label
        header_layout.addWidget(self.add_row_button)

        # Add the header layout to the main layout
        bottom_section_layout.addLayout(header_layout)

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
        self.target_list_display.setHorizontalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.target_list_display.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)

        # Allow manual resizing of the columns (on the horizontal header)
        header.setSectionResizeMode(QHeaderView.Interactive)

        # Disable editing of table cells
        self.target_list_display.setEditTriggers(QAbstractItemView.NoEditTriggers)

        # Set selection mode to select entire rows when a cell is clicked
        self.target_list_display.setSelectionBehavior(QAbstractItemView.SelectRows)

        # Enable horizontal scrolling by adding the table to a scroll area
        scroll_area = QScrollArea()
        scroll_area.setWidget(self.target_list_display)
        scroll_area.setWidgetResizable(True)  # Ensure that the scroll area resizes with the window

        # Customize the scroll bars to make them large when visible
        scroll_area.setStyleSheet("""
            QScrollBar:vertical, QScrollBar:horizontal {
                border: 2px solid grey;
                background: #F0F0F0;
                width: 20px;
                height: 20px;
            }
            QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
                background: #FFCC40;
                border-radius: 10px;
            }
            QScrollBar::add-line, QScrollBar::sub-line {
                border: none;
                background: none;
            }
        """)

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

    def add_new_row(self):
        # Create the dialog for input
        dialog = QDialog(self.parent)
        dialog.setWindowTitle("Add New Target")

        # Create a layout for the dialog
        dialog_layout = QVBoxLayout()

        # Create fields for the dialog (Name, RA, Decl, and optional fields)
        fields = {
            "Name": QLineEdit(),
            "RA": QLineEdit(),
            "Decl": QLineEdit(),
            "Offset RA": QLineEdit(),
            "Offset Dec": QLineEdit(),
            "EXPTime": QLineEdit(),
            "Slitwidth": QLineEdit(),
            "Magnitude": QLineEdit(),
        }

        for field_name, field_widget in fields.items():
            label = QLabel(field_name)
            dialog_layout.addWidget(label)
            dialog_layout.addWidget(field_widget)

        # Create the OK and Cancel buttons
        button_box = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        dialog_layout.addWidget(button_box)

        # Handle the OK and Cancel actions
        button_box.accepted.connect(dialog.accept)
        button_box.rejected.connect(dialog.reject)

        dialog.setLayout(dialog_layout)

        # Show the dialog and wait for the result
        if dialog.exec_() == QDialog.Accepted:
            # Get the values from the dialog fields
            row_data = []
            for field_name, field_widget in fields.items():
                row_data.append(field_widget.text())

            # Add a new row to the table with the input data
            current_row_count = self.target_list_display.rowCount()
            self.target_list_display.insertRow(current_row_count)

            # Add data to the new row
            for column, value in enumerate(row_data):
                self.target_list_display.setItem(current_row_count, column, QTableWidgetItem(value))

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
                if header == 'RA':
                    ra = value  # Store the exposure time
                    print(f"Found RA: {exposure_time}")  # Print the found exposure time

                # Check if the header is 'Exposure Time' and extract its value
                if header == 'DECL':
                    dec = value  # Store the exposure time
                    print(f"Found DEC: {exposure_time}")  # Print the found exposure time

                # Check if the header is 'Exposure Time' and extract its value
                if header == 'EXPTIME':
                    exposure_time = value  # Store the exposure time
                    print(f"Found Exposure Time: {exposure_time}")  # Print the found exposure time
                    self.control_tab.exposure_time_box.setText(re.sub(r'[a-zA-Z\s]', '', exposure_time))

                # Check if the header is 'Slit Width' and extract its value
                if header == 'SLITWIDTH':
                    slit_width = value  # Store the slit width
                    print(f"Found Slit Width: {slit_width}")  # Print the found slit width
                    self.control_tab.slit_width_box.setText(re.sub(r'[a-zA-Z\s]', '', slit_width))

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
            # self.parent.logic_service.update_target_list_table(target_data)

            # Call to set the column widths (adjust them as needed)
            self.set_column_widths()

            if observation_id:
                # Store the observation_id in a class variable for later use when the "Go" button is clicked
                self.parent.current_observation_id = observation_id
                self.parent.current_ra = ra
                self.parent.current_dec = dec
                self.parent.current_offset_ra = offset_ra
                self.parent.current_offset_dec = offset_dec
            
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

            # Select the row after updating the table (to highlight it)
            self.target_list_display.selectRow(selected_row)
            
            slit_angle = self.logic_service.compute_parallactic_angle_astroplan(self.parent.current_ra, self.parent.current_dec)
            self.control_tab.slit_angle_box.setText(slit_angle)
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "OTMslitangle", slit_angle)

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
            250, 275, 175, 175, 200, 200, 175, 175, 200, 200, 175, 175, 175, 175, 300, 250, 250, 200, 200, 175, 175, 250
        ]

        # Get the number of columns in the table
        column_count = self.target_list_display.columnCount()

        # Ensure we don't exceed the number of available columns
        for col in range(column_count):
            # Use the width from the list, or a default width if the list is too short
            width = column_widths[col] if col < len(column_widths) else 100
            self.target_list_display.setColumnWidth(col, width)
            
    def create_second_column_top_half(self):
        """Create the top half of the second column with tabs: 'Planning' and 'ETC'"""

        # Create a QVBoxLayout to hold everything in the top half
        second_column_top_half_layout = QVBoxLayout()

        # Create a QTabWidget to hold the tabs (Planning and ETC)
        self.parent.tabs = QTabWidget()

        # Create the two tabs: Planning and ETC
        self.parent.planning_tab = QWidget()
        self.parent.etc = QWidget()

        # Add the tabs to the QTabWidget
        self.parent.tabs.addTab(self.parent.planning_tab, "Planning")
        self.parent.tabs.addTab(self.parent.etc, "ETC")

        # Set up the layout for the "Planning" tab
        planning_layout = QVBoxLayout()
        planning_group = self.create_planning_info_group()
        planning_layout.addWidget(planning_group)
        self.parent.planning_tab.setLayout(planning_layout)

        # Set up the layout for the "ETC" tab
        self.create_etc_tab()  # Dynamically populate the "ETC" tab layout

        # Add the QTabWidget to the second column's top half layout
        second_column_top_half_layout.addWidget(self.parent.tabs)

        # Set the layout for the second_column_top_half (containing the tabs)
        second_column_top_half = QWidget()
        second_column_top_half.setLayout(second_column_top_half_layout)

        # Optional: Set maximum size for the group if needed (can be adjusted depending on available space)
        second_column_top_half.setMaximumHeight(350)  # Adjust based on design
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
                    target_lists = []  # Set to empty list if not valids

        except Exception as e:
            # Handle any exception that occurs during fetching
            print(f"Error fetching target lists: {e}")
            target_lists = []  # Set to empty list if an error occurs

        # If target_lists is empty, we can show a fallback message or an empty list
        if not target_lists:
            target_lists = ["No Target Lists Available"]  # Fallback message or an empty list
            self.add_row_button.setEnabled(False)

        # Ensure the ComboBox is cleared before populating it
        if isinstance(self.target_list_name, QComboBox):
            self.target_list_name.clear()

            # Add the fetched or fallback target lists to the ComboBox
            for set_name in target_lists:
                self.target_list_name.addItem(set_name)

            # Add an option for creating a new target list
            self.target_list_name.addItem("Create a new target list")

            # Set the first item as the default selection (if available)
            if target_lists:
                self.target_list_name.setCurrentIndex(0)  # Set the first item as default

            # Connect the signal for user selection change
            self.target_list_name.currentIndexChanged.connect(self.on_target_set_changed)

    def create_new_target_list(self):
        """Handle creating a new target list, uploading CSV, and creating a new target set."""
        # Step 1: Open the file dialog to allow the user to select a CSV file
        file_dialog = QFileDialog(self.parent)  # Assuming `self.parent` is the parent widget
        file_dialog.setFileMode(QFileDialog.ExistingFiles)
        file_dialog.setNameFilter("CSV Files (*.csv)")

        if file_dialog.exec_() == QFileDialog.Accepted:
            file_path = file_dialog.selectedFiles()[0]  # Get the selected file path

            # Step 2: Let the user provide a new target set name
            target_set_name, ok = QInputDialog.getText(self.parent, "Enter Target Set Name", "Target Set Name:")
            if ok and target_set_name:
                # Step 3: Call the logic service to upload the CSV and associate it with a new target set
                self.logic_service.upload_csv_to_mysql(file_path, target_set_name)
                self.target_list_name.setCurrentText(target_set_name)  # Set the newly created target list as selected
                self.parent.reload_table()

    def on_target_set_changed(self):
        """Handle the target set change in the ComboBox."""
        # Get the selected SET_NAME (not the entire list of target sets)
        selected_set_name = self.target_list_name.currentText()
        self.add_row_button.setEnabled(True)

        if selected_set_name == "Create a new target list":
            self.target_list_name.clear()
            # Trigger the CSV upload process
            self.create_new_target_list()
        else:
            print(f"Selected SET_NAME: {selected_set_name}")
            self.logic_service.update_target_table_with_list(selected_set_name)


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


    def create_etc_tab(self):
        """Create the layout and components for the 'ETC' tab with aligned labels and input boxes."""

        # Main vertical layout for stacking all the components
        etc_layout = QVBoxLayout()
        etc_layout.setSpacing(12)
        etc_layout.setContentsMargins(5, 15, 15, 15)

        # Common dimensions
        label_width = 120
        widget_height = 40
        short_input_width = 80
        dropdown_width = 120
        range_input_width = 90

        # Helper function to create aligned labels
        def create_aligned_label(text):
            label = QLabel(text)
            label.setFixedWidth(label_width)  # Fixed width to align with the input fields
            label.setAlignment(Qt.AlignRight | Qt.AlignVCenter)
            return label

        # Function to create a row with label and input field(s)
        def create_input_row(label_text, widgets):
            row_layout = QHBoxLayout()
            label = create_aligned_label(label_text)
            row_layout.addWidget(label)
            for widget in widgets:
                row_layout.addWidget(widget)
            return row_layout

        # Row 0: Magnitude, Filter, and System
        self.magnitude_input = QLineEdit()
        self.magnitude_input.setMinimumWidth(short_input_width)
        self.magnitude_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        self.filter_dropdown = QComboBox()
        self.filter_dropdown.addItems(["U", "G", "R", "I"])
        self.filter_dropdown.setMinimumWidth(dropdown_width)
        self.filter_dropdown.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        self.system_field = QLineEdit("AB")
        self.system_field.setReadOnly(True)
        self.system_field.setMinimumWidth(short_input_width)
        self.system_field.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)

        input_row_0 = create_input_row("Magnitude:", [self.magnitude_input, self.filter_dropdown, self.system_field])
        etc_layout.addLayout(input_row_0)

        # Row 1: Sky Mag and SNR (labels in one line with input fields)
        sky_mag_label = create_aligned_label("Sky Mag:")
        self.sky_mag_input = QLineEdit()
        self.sky_mag_input.setMinimumWidth(short_input_width)
        self.sky_mag_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        snr_label = create_aligned_label("SNR:")
        self.snr_input = QLineEdit()
        self.snr_input.setMinimumWidth(short_input_width)
        self.snr_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        # Combine Sky Mag and SNR in a horizontal layout
        sky_mag_snr_layout = QHBoxLayout()
        sky_mag_snr_layout.setSpacing(10)
        sky_mag_snr_layout.addWidget(sky_mag_label)
        sky_mag_snr_layout.addWidget(self.sky_mag_input)
        sky_mag_snr_layout.addWidget(snr_label)
        sky_mag_snr_layout.addWidget(self.snr_input)

        # Add the combined layout to the main layout
        etc_layout.addLayout(sky_mag_snr_layout)

        # Row 2: Slit Width and Slit Width Dropdown (on the next row)
        slit_width_label = create_aligned_label("Slit Width:")
        self.slit_width_input = QLineEdit()
        self.slit_width_input.setMinimumWidth(short_input_width)
        self.slit_width_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        self.slit_dropdown = QComboBox()
        self.slit_dropdown.addItems(["SET", "LOSS", "SNR", "RES", "AUTO"])
        self.slit_dropdown.setMinimumWidth(dropdown_width)
        self.slit_dropdown.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        slit_width_layout = QHBoxLayout()
        slit_width_layout.setSpacing(10)
        slit_width_layout.addWidget(slit_width_label)
        slit_width_layout.addWidget(self.slit_width_input)
        slit_width_layout.addWidget(self.slit_dropdown)

        # Add the slit width row to the layout
        etc_layout.addLayout(slit_width_layout)

        # Row 3: Range and No Slicer (Range label next to input boxes)
        range_label = create_aligned_label("Range:")
        
        self.range_input_start = QLineEdit()
        self.range_input_start.setMinimumWidth(range_input_width)
        self.range_input_start.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        range_dash = QLabel("-")
        range_dash.setFixedWidth(10)

        self.range_input_end = QLineEdit()
        self.range_input_end.setMinimumWidth(range_input_width)
        self.range_input_end.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        self.no_slicer_checkbox = QCheckBox("No Slicer")
        self.no_slicer_checkbox.setFixedHeight(widget_height)

        range_layout = QHBoxLayout()
        range_layout.setSpacing(10)
        range_layout.addWidget(range_label)  # Add the Range label to the same line
        range_layout.addWidget(self.range_input_start)
        range_layout.addWidget(range_dash)
        range_layout.addWidget(self.range_input_end)
        range_layout.addWidget(self.no_slicer_checkbox)

        etc_layout.addLayout(range_layout)

        # Row 4: Seeing (arcsec) and Airmass in one line
        seeing_label = create_aligned_label("Seeing (arcsec):")
        self.seeing_input = QLineEdit()
        self.seeing_input.setMinimumWidth(short_input_width)
        self.seeing_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        airmass_label = create_aligned_label("Airmass:")
        self.airmass_input = QLineEdit()
        self.airmass_input.setMinimumWidth(short_input_width)
        self.airmass_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        # Create Seeing and Airmass layout
        seeing_airmass_layout = QHBoxLayout()
        seeing_airmass_layout.setSpacing(10)  # Add spacing between widgets

        seeing_airmass_layout.addWidget(seeing_label)
        seeing_airmass_layout.addWidget(self.seeing_input)
        seeing_airmass_layout.addWidget(airmass_label)
        seeing_airmass_layout.addWidget(self.airmass_input)

        # Add the Seeing and Airmass input fields to the main layout
        etc_layout.addLayout(seeing_airmass_layout)

        # Row 5: EXPTIME and Resolution (same row with labels)
        exptime_label = create_aligned_label("EXPTIME:")
        self.exptime_input = QLineEdit()
        self.exptime_input.setMinimumWidth(short_input_width)
        self.exptime_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        resolution_label = create_aligned_label("Resolution:")
        self.resolution_input = QLineEdit()
        self.resolution_input.setMinimumWidth(short_input_width)
        self.resolution_input.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        # Create EXPTIME and Resolution layout
        exptime_resolution_layout = QHBoxLayout()
        exptime_resolution_layout.setSpacing(10)  # Add spacing between widgets

        exptime_resolution_layout.addWidget(exptime_label)
        exptime_resolution_layout.addWidget(self.exptime_input)
        exptime_resolution_layout.addWidget(resolution_label)
        exptime_resolution_layout.addWidget(self.resolution_input)

        # Add the EXPTIME and Resolution input fields to the main layout
        etc_layout.addLayout(exptime_resolution_layout)

        # Divider line between the two columns
        divider_line = QFrame()
        divider_line.setFrameShape(QFrame.HLine)  # Horizontal divider line
        divider_line.setFrameShadow(QFrame.Sunken)

        # Add the divider line to the layout
        etc_layout.addWidget(divider_line)

        # Buttons in the layout
        button_row_layout = QHBoxLayout()
        button_row_layout.setSpacing(10)

        run_button = QPushButton("Run ETC")
        run_button.setFixedSize(100, widget_height)
        run_button.clicked.connect(self.run_etc)

        save_button = QPushButton("Save")
        save_button.setFixedSize(100, widget_height)
        save_button.clicked.connect(self.save_etc)

        button_row_layout.addWidget(run_button)
        button_row_layout.addWidget(save_button)

        etc_layout.addLayout(button_row_layout)

        # Set the layout for the ETC tab
        self.parent.etc.setLayout(etc_layout)

        # Add a spacer to make sure widgets aren't squished
        spacer = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding)
        etc_layout.addItem(spacer)


    def validate_inputs(self):
        """Validates user inputs in the ETC tab and highlights invalid fields."""
        
        # Helper function to check if the input is a valid float and not empty
        def is_valid_float(text):
            if text.strip() == '':  # Check if the text is empty
                return False
            try:
                float(text)  # Try to convert to float
                return True
            except ValueError:
                return False  # Return False if the conversion fails

        # Reset all fields to default state (clear previous error highlighting)
        self.magnitude_input.setStyleSheet("")
        self.sky_mag_input.setStyleSheet("")
        self.snr_input.setStyleSheet("")
        self.slit_width_input.setStyleSheet("")
        self.range_input_start.setStyleSheet("")
        self.range_input_end.setStyleSheet("")
        
        try:
            # Check if all numeric inputs are valid
            if not is_valid_float(self.magnitude_input.text()):
                self.magnitude_input.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("Magnitude must be a valid number.")
            magnitude = float(self.magnitude_input.text())

            if not is_valid_float(self.sky_mag_input.text()):
                self.sky_mag_input.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("Sky Mag must be a valid number.")
            sky_mag = float(self.sky_mag_input.text())

            if not is_valid_float(self.snr_input.text()):
                self.snr_input.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("SNR must be a valid number.")
            snr = float(self.snr_input.text())

            if not is_valid_float(self.slit_width_input.text()):
                self.slit_width_input.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("Slit Width must be a valid number.")
            slit_width = float(self.slit_width_input.text())

            if not is_valid_float(self.range_input_start.text()):
                self.range_input_start.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("Range Start must be a valid number.")
            range_start = float(self.range_input_start.text())

            if not is_valid_float(self.range_input_end.text()):
                self.range_input_end.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("Range End must be a valid number.")
            range_end = float(self.range_input_end.text())
            
            # Ensure range_start is less than range_end
            if range_start >= range_end:
                self.range_input_start.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                self.range_input_end.setStyleSheet("border: 1px solid red;")  # Highlight invalid field
                raise ValueError("Range start must be less than range end.")

            # Check for valid values (you can adjust this for your specific needs)
            if magnitude <= 0 or sky_mag <= 0 or snr <= 0 or slit_width <= 0:
                raise ValueError("Magnitude, Sky Mag, SNR, and Slit Width must be positive values.")
            
            return True  # All inputs are valid

        except ValueError as e:
            # Show error message
            error_msg = f"Invalid input: {str(e)}"
            print(error_msg)
            return False  # Input is invalid


    def run_etc(self):
        """Handles the logic for the 'Run ETC' button."""
        
        # Validate inputs before running the command
        if not self.validate_inputs():
            return  # If inputs are invalid, do not proceed
        
        # Collecting all necessary data from input fields
        filter_value = self.filter_dropdown.currentText()  # e.g., "G"
        magnitude_value = self.magnitude_input.text()  # e.g., "18.0"
        sky_mag_value = self.sky_mag_input.text()  # e.g., "21.4"
        snr_value = self.snr_input.text()  # e.g., "10"
        slit_width_value = self.slit_width_input.text()  # e.g., "0.5"
        slit_option = self.slit_dropdown.currentText()  # e.g., "SET X"
        seeing_value = "1"  # Assuming a fixed value for seeing, update as needed
        airmass_value = "1"  # Assuming a fixed value for airmass, update as needed
        mag_system_value = self.system_field.text()  # e.g., "AB"
        mag_filter_value = self.mag_filter_dropdown.currentText()  # e.g., "match"
        
        # Handling the range inputs
        range_start_value = self.range_input_start.text()
        range_end_value = self.range_input_end.text()

        # Construct the command string
        command = f"python3 ETC/ETC_main.py {filter_value} {range_start_value} {range_end_value} SNR {snr_value} " \
                f"-slit {slit_option} {slit_width_value} -seeing {seeing_value} 500 -airmass {airmass_value} " \
                f"-skymag {sky_mag_value} -mag {magnitude_value} -magsystem {mag_system_value} -magfilter {mag_filter_value}"

        # Print the command for debugging
        print(f"Running command: {command}")
        
        # Run the command and capture the output
        try:
            result = subprocess.run(command, shell=True, capture_output=True, text=True)
            output = result.stdout.strip()  # Get the output from the command
            print(f"Command output: {output}")

            # Extract EXPTIME and RESOLUTION from the output using regex
            exptime_match = re.search(r"EXPTIME=([0-9.]+) s", output)
            resolution_match = re.search(r"RESOLUTION=([0-9.]+)", output)

            if exptime_match:
                exptime = float(exptime_match.group(1))
                exptime_rounded = round(exptime)  # Round EXPTIME to the nearest integer
                self.exptime_input.setText(str(exptime_rounded))  # Update GUI field with rounded EXPTIME

            if resolution_match:
                resolution = float(resolution_match.group(1))
                resolution_rounded = round(resolution)  # Round RESOLUTION to the nearest integer
                self.resolution_input.setText(str(resolution_rounded))  # Update GUI field with rounded RESOLUTION

        except subprocess.CalledProcessError as e:
            # Handle errors if the command fails
            print(f"Error running ETC: {e}")
        
        # Display the result in the results display (GUI)
        result_text = f"Running ETC with the following parameters:\n{command}\n\n" \
                    f"EXPTIME: {self.exptime_input.text()}\n" \
                    f"RESOLUTION: {self.resolution_input.text()}"
        print(result_text)

    def save_etc(self):
        exptime = self.exptime_input.text()
        resolution = self.resolution_input.text()
        if (self.parent.current_observation_id):
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "OTMexpt", exptime)
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "exptime", exptime)
            self.logic_service.send_update_to_db(self.parent.current_observation_id, "OTMres", resolution)
