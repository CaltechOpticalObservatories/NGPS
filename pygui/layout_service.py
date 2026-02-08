from PyQt5.QtWidgets import QVBoxLayout, QAbstractItemView, QStyle, QFrame, QDialog, QListView, QFileDialog, QDialogButtonBox, QMessageBox,  QInputDialog, QHBoxLayout, QGridLayout, QTableWidget, QHeaderView, QFormLayout, QListWidget, QListWidgetItem, QScrollArea, QVBoxLayout, QGroupBox, QGroupBox, QHeaderView, QLabel, QRadioButton, QProgressBar, QLineEdit, QTextEdit, QTableWidget, QComboBox, QDateTimeEdit, QTabWidget, QWidget, QPushButton, QCheckBox,QSpacerItem, QSizePolicy
from PyQt5.QtCore import QDateTime, QTimer
from PyQt5.QtGui import QColor, QFont, QDoubleValidator
from logic_service import LogicService
from PyQt5.QtCore import Qt, QSignalBlocker
from control_tab import ControlTab
from instrument_status_tab import InstrumentStatusTab
from database_tab import DatabaseTab
import re

class LayoutService:
    def __init__(self, parent):
        self.parent = parent
        self.logic_service = LogicService(self.parent)
        self.target_list_display = None 
        self.target_list_name = QComboBox()
        self.target_list_name.setView(QListView())
        self.target_list_name.setMaxVisibleItems(16)
        self.target_list_name.view().setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.add_row_button = QPushButton()
        self.save_button = QPushButton()
        self.lamp_checkboxes = {}
        self.modulator_checkboxes = {}

        # Create the control tab instance
        self.control_tab = ControlTab(self.parent)

        # Create the instrument status tab instance
        self.instrument_status_tab = InstrumentStatusTab(self.parent)

        # Create the database tab instance
        self.database_tab = None  # Will be initialized after DB connection

        
    def get_screen_size_ratio(self):
        # Get the user's screen size
        # screen = self.parent.screen()
        # screen_size = screen.geometry()
        
        # Use the screen width and height to calculate dynamic ratio (you can adjust these ratios)
        screen_width = 800
        screen_height = 600

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

        # Create the QTabWidget and the Control tab
        self.parent.tabs = QTabWidget()
        self.parent.control_tab = QWidget()
        self.parent.status_tab = QWidget()
        self.parent.database_tab = QWidget()  # Keep for legacy compatibility but don't add to tabs
        self.parent.engineering_tab = QWidget()

        # Add the tabs to the QTabWidget
        self.parent.tabs.addTab(self.parent.control_tab, "Control")
        # DatabaseTab widget is now in the center column, not in right side panel
        # self.parent.tabs.addTab(self.parent.database_tab, "Target Sets")
        # self.parent.tabs.addTab(self.parent.status_tab, "Status")

        # Add the QTabWidget to the third column layout
        third_column_layout.addWidget(self.parent.tabs)

        # Set up the layout for the Control tab
        self.control_tab = ControlTab(self.parent)
        control_layout = QVBoxLayout()
        control_layout.addWidget(self.control_tab)
        self.parent.control_tab.setLayout(control_layout)

         # Create the Instrument tab
        self.status_tab = InstrumentStatusTab(self.parent)
        status_layout = QVBoxLayout()
        status_layout.addWidget(self.status_tab)
        self.parent.status_tab.setLayout(status_layout)

        # Database tab will be initialized separately after DB connection
        database_layout = QVBoxLayout()
        self.parent.database_tab.setLayout(database_layout)

        return third_column_layout

    def initialize_database_tab(self):
        """Initialize the database tab after DB connection is established.

        Note: DatabaseTab is now created directly in create_target_list_group()
        with its own DB connection. This method is kept for compatibility.
        """
        # DatabaseTab already created in create_target_list_group()
        if self.database_tab_widget:
            print("Database tab already initialized")
        else:
            print("Warning: Database tab widget not found")

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
        system_status_group.setContentsMargins(0, 45, 0, 0) 
        left_top_layout.addWidget(system_status_group)

        # TCS Status
        tcs_status_group = self.create_tcs_status_group()
        left_top_layout.addWidget(tcs_status_group)
        
        # Sequencer Mode
        sequencer_mode_group = self.create_sequencer_mode_group()
        left_top_layout.addWidget(sequencer_mode_group)
        
        # Target List Dropdown Mode
        target_dropdown_group = self.create_target_dropdown_group()
        left_top_layout.addWidget(target_dropdown_group)

        return left_top_layout

    def create_right_top_layout(self):
        right_top_layout = QVBoxLayout()
        right_top_layout.setContentsMargins(15, 0, 0, 15) 

        # Progress and Image Info Group
        progress_and_image_group = self.create_progress_and_image_group()
        right_top_layout.addWidget(progress_and_image_group)

        return right_top_layout

    def create_system_status_group(self):
        system_status_group = QGroupBox("Instrument System Status")
        system_status_layout = QVBoxLayout()
        system_status_layout.setSpacing(10)
        system_status_layout.setContentsMargins(5, 5, 5, 5)

        # Create a mapping for status colors
        status_map = {
            "stopped": QColor(169, 169, 169),  # Grey
            "idle": QColor(255, 255, 0),       # Yellow
            "paused": QColor(255, 165, 0),     # Orange
            "exposing": QColor(0, 255, 0),     # Green
            "readout": QColor(0, 255, 0),      # Green
            "acquire": QColor(255, 255, 0),    # Yellow           
            "focus": QColor(255, 255, 0),      # Yellow     
            "calib": QColor(255, 255, 0),      # Yellow     
            "user": QColor(255, 255, 0),      # Yellow     
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

            # Hide the widget initially (default is 'stopped')
            status_widget.setVisible(False)

        system_status_group.setLayout(system_status_layout)

        # Set maximum width and height for the system status group
        system_status_group.setMaximumWidth(300)  # Maximum width
        system_status_group.setMaximumHeight(250)  # Maximum height

        # Ensure only the 'stopped' status is fully active by default
        self.update_system_status("stopped")

        return system_status_group

    def update_system_status(self, status):
        """
        Update the system status and make only the relevant widget visible.
        Hide all other status widgets.
        """
        # Hide all status widgets
        for status_key, status_info in self.status_widgets.items():
            status_info['widget'].setVisible(False)

        # Show the widget corresponding to the current status
        if status in self.status_widgets:
            self.status_widgets[status]['widget'].setVisible(True)

    def create_tcs_status_group(self):
        tcs_status_group = QGroupBox("TCS Status")
        tcs_status_layout = QVBoxLayout()
        tcs_status_layout.setSpacing(3)  # Reduced space between items in the layout
        tcs_status_layout.setContentsMargins(5, 5, 5, 5)  # Reduced margins around the layout

        # Create a mapping for status colors
        tcs_status_map = {
            "idle": QColor(255, 255, 0),       # Yellow
            "on": QColor(0, 255, 0),           # Green
            "tracking": QColor(0, 255, 0),    # Green
            "paused": QColor(255, 165, 0),     # Orange
            "error": QColor(255, 0, 0),        # Red
        }

        # Create a dictionary to hold the status widgets, which we will enable/disable
        self.tcs_status_widgets = {}

        # Create status widgets and add them to the layout
        for status, color in tcs_status_map.items():
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
            tcs_status_layout.addWidget(status_widget)

            # Store status widgets in a dictionary for later use
            self.tcs_status_widgets[status] = {
                'widget': status_widget,
                'color_rect': status_color_rect,
                'label': status_label,
                'color': color
            }

            # Hide the widget initially (default is 'idle')
            status_widget.setVisible(False)

        tcs_status_group.setLayout(tcs_status_layout)

        # Set maximum width and height for the TCS status group
        tcs_status_group.setMaximumWidth(300)  # Maximum width
        tcs_status_group.setMaximumHeight(250)  # Maximum height

        # Ensure only the 'idle' status is fully active by default
        self.update_tcs_status("on")

        return tcs_status_group

    def update_tcs_status(self, status):
        """Update the TCS status and make the appropriate widget visible."""
        # Hide all widgets initially
        for status_key, status_info in self.tcs_status_widgets.items():
            status_info['widget'].setVisible(False)

        # Show the widget corresponding to the current status
        if status in self.tcs_status_widgets:
            self.tcs_status_widgets[status]['widget'].setVisible(True)

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
        progress_and_image_layout.setSpacing(15)

        # Create the progress layout with separate rows for exposure and overhead
        progress_layout = self.create_progress_layout()

        # Create the image info layout and message log as before
        image_info_layout = self.create_image_info_layout()
        message_log = self.create_message_log()

        # Add all components to the main layout
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
        progress_layout = QVBoxLayout()

        # Exposure Progress
        exposure_layout = QHBoxLayout()

        self.parent.exposure_progress = QProgressBar()
        self.parent.exposure_progress.setRange(0, 100)
        self.parent.exposure_progress.setValue(0)
        self.parent.exposure_progress.setMaximumWidth(600)
        self.parent.exposure_progress.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.parent.exposure_progress.setTextVisible(True)
        self.parent.exposure_progress.setFormat("0%")

        exposure_layout.setSpacing(5)
        exposure_layout.addWidget(QLabel("Exposure Progress:"))
        exposure_layout.addWidget(self.parent.exposure_progress)

        # Readout/Overhead Progress
        overhead_layout = QHBoxLayout()  # Horizontal layout for overhead row

        self.parent.overhead_progress = QProgressBar()
        self.parent.overhead_progress.setValue(0)
        self.parent.overhead_progress.setRange(0, 100)
        self.parent.overhead_progress.setMaximumWidth(300)
        self.parent.overhead_progress.setTextVisible(True)

        overhead_layout.setSpacing(0)
        overhead_layout.addWidget(QLabel("Readout Progress:"))
        overhead_layout.addSpacing(8) 
        overhead_layout.addWidget(self.parent.overhead_progress)

        self.parent.shutter_label = QLabel("Shutter:")
        self.parent.shutter_label.setAlignment(Qt.AlignVCenter | Qt.AlignRight)

        self.parent.shutter_box = QLabel("CLOSED")
        self.parent.shutter_box.setAlignment(Qt.AlignCenter)
        self.parent.shutter_box.setFixedWidth(90)
        self.parent.shutter_box.setStyleSheet("""
            border: 1px solid gray;
            border-radius: 6px;
            padding: 2px 6px;
            background-color: #ccc;
            color: black;
        """)

        overhead_layout.addSpacing(12)
        overhead_layout.addWidget(self.parent.shutter_label)
        overhead_layout.addSpacing(6) 
        overhead_layout.addWidget(self.parent.shutter_box)

        # Stack both layouts
        progress_layout.addLayout(exposure_layout)
        progress_layout.addLayout(overhead_layout)

        return progress_layout

    def update_exposure_progress(self, progress_percentage, remaining_sec):
        """Update the exposure progress bar with percentage and remaining time."""
        label_text = f"{progress_percentage}% ({remaining_sec} sec remaining)"
        self.parent.exposure_progress.setValue(progress_percentage)
        self.parent.exposure_progress.setFormat(label_text)

    def update_readout_progress(self, progress_percentage):
        """Update the readout progress bar based on the received percentage."""
        self.parent.overhead_progress.setValue(progress_percentage)

    def update_shutter_status(self, is_open):
        if isinstance(is_open, str):
            up = is_open.strip().upper()
            is_open = True if up in ("OPEN", "OPENED", "O") else False if up in ("CLOSE","CLOSED","C") else False

        if is_open:
            self.parent.shutter_box.setText("OPEN")
            self.parent.shutter_box.setStyleSheet(
                "border: 1px solid gray; padding: 2px; background-color: #cccccc; color: black;"
            )
        else:
            self.parent.shutter_box.setText("CLOSED")
            self.parent.shutter_box.setStyleSheet(
                "border: 1px solid gray; padding: 2px; background-color: #cccccc; color: black;"
            )


    def create_image_info_layout(self):
        image_info_layout = QHBoxLayout()
        image_info_layout.setSpacing(10)

        # Create the QLineEdit widgets
        self.parent.image_name = QLineEdit("N/A")
        self.parent.image_name.setReadOnly(True)
        self.parent.image_number = QLineEdit("N/A")
        self.parent.image_number.setReadOnly(True)

        # Set the image_name widget to stretch and fill available space
        self.parent.image_name.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        # Set the image_number widget to be smaller
        self.parent.image_number.setFixedWidth(80)

        image_info_layout.addWidget(QLabel("Image Dir:"))
        image_info_layout.addWidget(self.parent.image_name)
        image_info_layout.addWidget(QLabel("Image Number:"))
        image_info_layout.addWidget(self.parent.image_number)

        return image_info_layout

    def update_image_number(self, image_number):
        """Update the current image number."""
        self.parent.image_number.setText(str(image_number))

    def update_image_name(self, image_name):
        """Update the current image name."""
        self.parent.image_name.setText(str(image_name))

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
        main_layout = QVBoxLayout()
        main_layout.setSpacing(0)
        main_layout.setContentsMargins(2, 2, 2, 2)

        # ── Create DatabaseTab directly (it manages its own DB connection) ──
        try:
            self.database_tab_widget = DatabaseTab(target_list_group, None, main_window=self.parent)
            main_layout.addWidget(self.database_tab_widget, 1)
            print("Database tab created successfully")
        except Exception as exc:
            print(f"Failed to create database tab: {exc}")
            import traceback
            traceback.print_exc()
            # Fallback: show error label
            error_label = QLabel(f"Database tab failed to load: {exc}")
            main_layout.addWidget(error_label)

        # ── Legacy attributes (kept for backward compatibility, never shown) ──
        self.database_tab_container = QWidget()
        self.database_tab_container.setVisible(False)
        self.target_list_display = QTableWidget()
        self.target_list_display.setRowCount(0)
        self.target_list_display.setColumnCount(0)
        self.target_list_display.setVisible(False)
        self.load_target_button = QPushButton()
        self.load_target_button.setVisible(False)
        self.add_row_button = QPushButton()
        self.add_row_button.setVisible(False)
        self.column_toggle_button = QPushButton()
        self.column_toggle_button.setVisible(False)

        target_list_group.setLayout(main_layout)
        return target_list_group

    def show_column_toggle_dialog(self):
        table = self.target_list_display

        # If there are no columns yet, bail out nicely
        if table.columnCount() == 0:
            QMessageBox.information(
                self.parent,
                "No columns",
                "There are no target list columns to toggle yet."
            )
            return

        dialog = QDialog(self.parent)
        dialog.setWindowTitle("Show / hide target list fields")

        layout = QVBoxLayout(dialog)

        # Build a checkbox for each column, based on header text
        checkboxes = []
        for col in range(table.columnCount()):
            header_item = table.horizontalHeaderItem(col)
            header_text = header_item.text() if header_item else f"Column {col + 1}"

            cb = QCheckBox(header_text)
            # Checked == visible
            cb.setChecked(not table.isColumnHidden(col))
            layout.addWidget(cb)
            checkboxes.append((col, cb))

        # OK/Cancel buttons
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        layout.addWidget(buttons)
        buttons.accepted.connect(dialog.accept)
        buttons.rejected.connect(dialog.reject)

        if dialog.exec_() != QDialog.Accepted:
            return  # user cancelled

        # Apply the chosen visibility
        for col, cb in checkboxes:
            table.setColumnHidden(col, not cb.isChecked())

        # Optionally re-apply your column widths (visible ones keep their widths)
        self.set_column_widths()

    def hide_default_columns(self):
        """Hide noisy / advanced columns in the target list by default."""
        # Make this safe if called before the table is created
        table = getattr(self, "target_list_display", None)
        if table is None:
            return

        if table.columnCount() == 0:
            return

        # Column names to hide by default
        to_hide = {"OBSERVATION_ID", "CHANNEL", "MAGNITUDE", "MAGSYSTEM", "MAGFILTER"}

        for col in range(table.columnCount()):
            header_item = table.horizontalHeaderItem(col)
            if header_item is None:
                continue

            header_text = header_item.text().strip().upper()
            if header_text in to_hide:
                table.setColumnHidden(col, True)

    def on_row_selected(self):
        # Get the selected row's index
        selected_rows = self.parent.target_list_display.selectionModel().selectedRows()
        if selected_rows:
            selected_row = selected_rows[0].row()  # Get the first selected row (you can handle multi-row selection here)
            # Pass the selected row to LogicService
            self.parent.logic_service.update_target_information(selected_row)

    def add_new_row(self):
        # Dialog
        dialog = QDialog(self.parent)
        dialog.setWindowTitle("Add New Target")
        layout = QVBoxLayout(dialog)

        form = QFormLayout()
        layout.addLayout(form)

        # Fields
        name_le      = QLineEdit()
        ra_le        = QLineEdit()
        decl_le      = QLineEdit()
        off_ra_le    = QLineEdit()
        off_dec_le   = QLineEdit()
        exptime_le   = QLineEdit()
        slitwidth_le = QLineEdit()
        mag_le       = QLineEdit()

        # Placeholders
        name_le.setPlaceholderText("e.g. NGC 1234")
        ra_le.setPlaceholderText("e.g. 12 34 56.7  or  188.736 (deg)")
        decl_le.setPlaceholderText("e.g. +12 34 56  or  +12.582 (deg)")
        off_ra_le.setPlaceholderText("arcsec (optional)")
        off_dec_le.setPlaceholderText("arcsec (optional)")
        exptime_le.setPlaceholderText("seconds")
        slitwidth_le.setPlaceholderText("arcsec")
        mag_le.setPlaceholderText("e.g. 17.2 (optional)")

        # Numeric validators (optional, allow blanks)
        dv = QDoubleValidator()
        for w in (off_ra_le, off_dec_le, exptime_le, slitwidth_le, mag_le):
            w.setValidator(dv)

        # Required labels
        form.addRow(QLabel("<b>Name*</b>"), name_le)
        form.addRow(QLabel("<b>RA*</b>"), ra_le)
        form.addRow(QLabel("<b>Decl*</b>"), decl_le)
        form.addRow(QLabel("<b>EXPTime*</b>"), exptime_le)
        form.addRow(QLabel("<b>Slitwidth*</b>"), slitwidth_le)
        form.addRow("Offset RA", off_ra_le)
        form.addRow("Offset Dec", off_dec_le)
        form.addRow("Magnitude", mag_le)

        # Buttons
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        layout.addWidget(buttons)
        buttons.accepted.connect(dialog.accept)
        buttons.rejected.connect(dialog.reject)

        if dialog.exec_() != QDialog.Accepted:
            return

        # Read & basic validate
        target_name = name_le.text().strip()
        ra          = ra_le.text().strip()
        decl        = decl_le.text().strip()
        exp        = exptime_le.text().strip()
        slitwidth        = slitwidth_le.text().strip()

        if not target_name or not ra or not decl or not exp or not slitwidth:
            QMessageBox.warning(self.parent, "Missing fields", "Name, RA, Decl, Exptime, Slitwidth are required.")
            return

        # Convert numeric optionals (blank -> None)
        def _num(txt):
            txt = txt.strip()
            return float(txt) if txt else None

        try:
            offset_ra  = _num(off_ra_le.text())
            offset_dec = _num(off_dec_le.text())
            exptime    = "SET " + exptime_le.text()
            slitwidth  = "SET " + slitwidth_le.text()
            magnitude  = _num(mag_le.text())
        except ValueError:
            QMessageBox.warning(self.parent, "Invalid number", "One or more numeric fields are invalid.")
            return

        # Ensure we have a current SET_ID
        set_id = self.logic_service.fetch_set_id(getattr(self.parent, "current_target_list_name", None))
        if set_id is None:
            QMessageBox.warning(self.parent, "No target list selected", "Please select a valid target list first.")
            return

        # Insert into DB
        self.logic_service.insert_target_to_db(
            target_name, ra, decl, offset_ra, offset_dec, exptime, slitwidth, magnitude
        )

        # Refresh the table for the currently selected set (DB-backed fetch)
        if hasattr(self.logic_service, "filter_target_list"):
            self.logic_service.filter_target_list()


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
            num_of_exposures = None

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
                    ra = value  # Store the ra
                    print(f"Found RA: {ra}")  # Print the found ra

                # Check if the header is 'Exposure Time' and extract its value
                if header == 'DECL':
                    dec = value  # Store the dec
                    print(f"Found DEC: {dec}")  # Print the found dec

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

                # Check if the header is 'BINSPECT' and extract its value
                if header == 'BINSPECT':
                    binspect = value  
                    print(f"Found BINSPECT: {binspect}") 
                    self.control_tab.bin_spect_box.setText(binspect)

                # Check if the header is 'BINSPAT' and extract its value
                if header == 'BINSPAT':
                    binspat = value 
                    print(f"Found BINSPAT: {binspat}")
                    self.control_tab.bin_spat_box.setText(binspat) 
                
                # If the header is 'NAME', store it as the target name
                if header == 'NAME':
                    target_name = value

                # Check if the header is 'NEXP' and extract its value
                if header == 'NEXP':
                    num_of_exposures = value  # Store the NEXP
                    print(f"Found NEXP: {num_of_exposures}")  # Print the found NEXP 
                    self.control_tab.num_of_exposures_box.setText(num_of_exposures)

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
                self.parent.num_of_exposures = num_of_exposures
                self.parent.current_bin_spect = binspect
                self.parent.current_bin_spat = binspat
            
            if target_name:
                self.control_tab.target_name_label.setText(f"Selected Target: {target_name}")
                self.control_tab.ra_dec_label.setText(f"RA: {ra}, Dec: {dec}")
            else:
                self.control_tab.setText("Selected Target: Not Selected")
                self.control_tab.ra_dec_label.setText(f"RA: Not Set, Dec: Not Set")

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
            self.control_tab.offset_to_target_button.setEnabled(False)
            self.control_tab.continue_button.setEnabled(False) 
            # Select the row after updating the table (to highlight it)
            self.target_list_display.selectRow(selected_row)
            
            slit_angle = "0"
            # if self.parent.current_ra != '' and self.parent.current_dec != '':
            #     slit_angle = self.logic_service.compute_parallactic_angle_astroplan(self.parent.current_ra, self.parent.current_dec)
            self.control_tab.slit_angle_box.setText(slit_angle)

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
            250, 175, 125, 125, 125, 125, 125, 125, 125, 125, 175, 175, 175, 175, 175, 175, 175, 175, 175, 125, 125, 125
        ]

        # Get the number of columns in the table
        column_count = self.target_list_display.columnCount()

        # Ensure we don't exceed the number of available columns
        for col in range(column_count):
            # Use the width from the list, or a default width if the list is too short
            width = column_widths[col] if col < len(column_widths) else 150
            self.target_list_display.setColumnWidth(col, width)


    def update_status_ui(self, data, modulator_data):
        """ Update the UI based on the received data and modulator data. """
        if not isinstance(data, dict) or not isinstance(modulator_data, dict):
            self.logger.error("Invalid data format received.")
            return

        # List of lamps to process
        lamps = ["LAMPBLUC", "LAMPFEAR", "LAMPREDC", "LAMPTHAR"]  # List of lamps in the message

        for lamp in lamps:
            # Update the lamp checkbox
            lamp_checkbox = self.lamp_checkboxes.get(lamp)
            if lamp_checkbox:
                # Set the checkbox state based on the value from the payload (True/False)
                lamp_checkbox.setChecked(data.get(lamp, False))  # Will be unchecked if not found

            # Update the modulator checkbox based on the corresponding modulator state in modulator_data
            modulator_checkbox = self.modulator_checkboxes.get(lamp)
            if modulator_checkbox:
                # Get the modulator status from modulator_data (e.g., MODBLCON, MODFEAR, etc.)
                modulator_key = f"MOD{lamp[4:]}"  # For LAMPBLUC, this would give "MODBLCON"
                
                modulator_status = modulator_data.get(modulator_key, "off")
                
                # Determine if the modulator is on or off based on the modulator status
                modulator_checkbox.setChecked(modulator_status.startswith("on"))  # "on" means checked, "off" means unchecked

    def update_slit_info_fields(self, slit_width=None, slit_offset=None):
        if slit_width is None or slit_offset is None:
            self.slit_width_input.setText("N/A")
        else:
            self.slit_width_input.setText(f"{slit_width:.3f} / {slit_offset:.3f}")

    def create_second_column_top_half(self):
        """Create the top half of the second column with 'Status', Calibration Lamps, and additional status fields"""
        
        second_column_top_half_layout = QVBoxLayout()

        # Create the "Status" section
        status_group = QGroupBox("Status")
        status_layout = QVBoxLayout()
        status_layout.setSpacing(10)

        # Create Calibration Lamps section
        calibration_lamps_group = QGroupBox("Calibration Lamps")
        calibration_lamps_layout = QVBoxLayout()

        # Define lamps and their corresponding statuses
        lamps = ["ThAR", "FeAr", "RedCont", "BlueCont"]

        # Create the header row for "Lamps On/Off" and "Modulator On/Off"
        header_layout = QHBoxLayout()

        lamps_header = QLabel("Lamps")
        lamps_header.setAlignment(Qt.AlignCenter)
        header_layout.addWidget(lamps_header)

        modulator_header = QLabel("Modulator")
        modulator_header.setAlignment(Qt.AlignCenter)
        header_layout.addWidget(modulator_header)

        calibration_lamps_layout.addLayout(header_layout)

        # For each lamp, create two separate HBoxes: one for Lamp and one for Modulator, with a separator bar between them
        for lamp in lamps:
            lamp_layout = QHBoxLayout()  # Main layout for each lamp's row

            lamp_side_layout = QHBoxLayout()
            lamp_name = QLabel(lamp)
            lamp_side_layout.addWidget(lamp_name)

            lamp_checkbox = QCheckBox("On/Off")
            lamp_checkbox.setChecked(False)  # Default to Off
            lamp_checkbox.setEnabled(False)
            lamp_side_layout.addWidget(lamp_checkbox)

            separator = QFrame()
            separator.setFrameShape(QFrame.VLine)  # Vertical line
            separator.setFrameShadow(QFrame.Sunken)
            separator.setStyleSheet("background-color: white;")
            separator.setLineWidth(1)

            modulator_side_layout = QHBoxLayout()
            modulator_checkbox = QCheckBox("On/Off")
            modulator_checkbox.setChecked(False)  # Default to Off
            modulator_checkbox.setEnabled(False)
            modulator_side_layout.addWidget(modulator_checkbox)

            lamp_layout.addLayout(lamp_side_layout)
            lamp_layout.addWidget(separator)
            lamp_layout.addLayout(modulator_side_layout)

            # Store checkboxes for later updates
            self.lamp_checkboxes[lamp] = lamp_checkbox
            self.modulator_checkboxes[lamp] = modulator_checkbox

            calibration_lamps_layout.addLayout(lamp_layout)

        # Add the Calibration Lamps section to the status layout
        calibration_lamps_group.setLayout(calibration_lamps_layout)
        status_layout.addWidget(calibration_lamps_group)

        # Create a horizontal layout to arrange Seeing and Airmass side by side
        first_row_layout = QHBoxLayout()

        # Seeing (arcsec) field
        seeing_layout = QVBoxLayout()
        seeing_label = QLabel("Seeing (arcsec):")
        self.seeing_input = QLineEdit()
        self.seeing_input.setReadOnly(True)  # Make it read-only
        self.seeing_input.setText("N/A")  # Placeholder text or dynamically updated value
        
        seeing_layout.addWidget(seeing_label)
        seeing_layout.addWidget(self.seeing_input)

        # Airmass field
        airmass_layout = QVBoxLayout()
        airmass_label = QLabel("Airmass:")
        self.airmass_input = QLineEdit()
        self.airmass_input.setReadOnly(True)  # Make it read-only
        self.airmass_input.setText("N/A")  # Placeholder text or dynamically updated value
        
        airmass_layout.addWidget(airmass_label)
        airmass_layout.addWidget(self.airmass_input)

        # Add the individual layouts to the first row layout
        first_row_layout.addLayout(seeing_layout)
        first_row_layout.addLayout(airmass_layout)

        # Add the first row layout to the status layout
        status_layout.addLayout(first_row_layout)

        # Create a second row layout for Binning and Slit Width Offset
        second_row_layout = QHBoxLayout()

        # Binning field
        binning_layout = QVBoxLayout()
        binning_label = QLabel("Binning:")
        self.binning_input = QLineEdit()
        self.binning_input.setReadOnly(True)  # Make it read-only
        self.binning_input.setText("N/A")  # Placeholder text or dynamically updated value
        
        binning_layout.addWidget(binning_label)
        binning_layout.addWidget(self.binning_input)

        # Slit Width Offset field
        slit_width_layout = QVBoxLayout()
        slit_width_label = QLabel("Slitwidth & Offset:")
        self.slit_width_input = QLineEdit()
        self.slit_width_input.setReadOnly(True)  # Make it read-only
        self.slit_width_input.setText("N/A")  # Placeholder text or dynamically updated value
        
        slit_width_layout.addWidget(slit_width_label)
        slit_width_layout.addWidget(self.slit_width_input)

        # Add the individual layouts to the second row layout
        second_row_layout.addLayout(binning_layout)
        second_row_layout.addLayout(slit_width_layout)

        # Add the second row layout to the status layout
        status_layout.addLayout(second_row_layout)

        # Set the status section layout and add it to the second column
        status_group.setLayout(status_layout)
        second_column_top_half_layout.addWidget(status_group)

        # Optional: Set maximum size for the group if needed (can be adjusted depending on available space)
        status_group.setMaximumHeight(350)  # Adjust based on design
        status_group.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)  # Allow vertical resizing if needed

        # Return the layout for the second column's top half
        second_column_top_half = QWidget()
        second_column_top_half.setLayout(second_column_top_half_layout)

        return second_column_top_half

    def update_airmass(self, airmass):
        """Update the airmass based on the incoming TCS status."""
        self.airmass_input.setText(str(airmass))


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
        left_planning_column.setSpacing(10)
        left_planning_column.setContentsMargins(0, 0, 0, 0)

        # Create the Start Date & Time edit
        self.parent.start_date_time_edit = QDateTimeEdit()
        self.parent.start_date_time_edit.setDateTime(QDateTime.currentDateTime())
        self.parent.start_date_time_edit.setDisplayFormat("MM/dd/yyyy h:mm AP")
        self.parent.start_date_time_edit.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        # Seeing input
        self.parent.seeing = QLineEdit("1.0")
        self.parent.seeing.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        # Airmass limit input
        self.parent.airmass_limit = QLineEdit("2.0")
        self.parent.airmass_limit.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        # Target List Type dropdown (NEW)
        target_list_type_layout = QHBoxLayout()
        target_list_type_label = QLabel("Target List Type")
        target_list_type_label.setMaximumHeight(40)

        self.target_list_type_dropdown = QComboBox()
        self.target_list_type_dropdown.addItems(["Science", "Calibration"])
        self.target_list_type_dropdown.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        # For now, just print when it changes
        self.target_list_type_dropdown.currentIndexChanged.connect(
            lambda: print(f"Target List Type switched to: {self.target_list_type_dropdown.currentText()}")
        )

        target_list_type_layout.addWidget(target_list_type_label)
        target_list_type_layout.addWidget(self.target_list_type_dropdown)

        # Target List name dropdown
        self.target_list_name.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        # Start Date layout
        start_date_layout = QVBoxLayout()
        label = QLabel("Start Date & Time (PST)")
        label.setMaximumHeight(40)
        start_date_layout.addWidget(label)
        start_date_layout.addWidget(self.parent.start_date_time_edit)
        start_date_layout.setAlignment(Qt.AlignLeft)

        # Seeing layout
        seeing_layout = QHBoxLayout()
        seeing_layout.addWidget(QLabel("Seeing (arcsec)"))
        seeing_layout.addWidget(self.parent.seeing)
        seeing_layout.setAlignment(Qt.AlignLeft)

        # Airmass layout
        airmass_layout = QHBoxLayout()
        airmass_layout.addWidget(QLabel("Airmass Limit"))
        airmass_layout.addWidget(self.parent.airmass_limit)
        airmass_layout.setAlignment(Qt.AlignLeft)

        # Target List layout
        target_list_layout = QVBoxLayout()
        target_label = QLabel("Target List")
        target_label.setMaximumHeight(40)
        target_list_layout.addWidget(target_label)
        target_list_layout.addWidget(self.target_list_name)
        target_list_layout.setAlignment(Qt.AlignLeft)

        # Add layouts to the main column
        left_planning_column.addLayout(start_date_layout)
        left_planning_column.addLayout(seeing_layout)
        left_planning_column.addLayout(airmass_layout)
        left_planning_column.addLayout(target_list_type_layout)
        left_planning_column.addLayout(target_list_layout)

        # Load target lists when first created
        self.load_target_lists()

        return left_planning_column
    
    def load_target_lists(self, target_lists=None):
        """Populate the ComboBox with target lists, switching between Science and Calibration modes."""
        try:
            if self.target_list_mode_toggle.isChecked():
                # Calibration mode
                print("Loading Calibration target lists...")
                target_lists = self.logic_service.load_calibration_target_sets("config/db_config.ini")

                if not isinstance(target_lists, (list, tuple)):
                    print("Error: Calibration data is not a valid iterable.")
                    target_lists = []

            else:
                # Science mode
                print("Loading Science target lists...")
                if target_lists is None:
                    target_lists = self.logic_service.load_mysql_and_fetch_target_sets("config/db_config.ini")

                    if not isinstance(target_lists, (list, tuple)):
                        print("Error: Fetched data is not a valid iterable.")
                        target_lists = []

        except Exception as e:
            print(f"Error fetching target lists: {e}")
            target_lists = []

        # Fallback if no data is found
        if not target_lists:
            target_lists = ["No Target Lists Available"]
            self.add_row_button.setEnabled(False)
        else:
            self.add_row_button.setEnabled(True)

        # Populate the combo box
        if isinstance(self.target_list_name, QComboBox):
            self.target_list_name.blockSignals(True)
            self.target_list_name.clear()

            for set_name in target_lists:
                self.target_list_name.addItem(set_name)

            # Sentinels at the end
            self.target_list_name.addItem("Upload new target list")
            self.target_list_name.addItem("Create empty target list")

            # Prefer selecting what LogicService marked as current
            preferred = getattr(self.parent, "current_target_list_name", None)
            if preferred:
                idx = self.target_list_name.findText(str(preferred))
                if idx >= 0:
                    self.target_list_name.setCurrentIndex(idx)
                else:
                    self.target_list_name.setCurrentIndex(0 if target_lists else -1)
            else:
                self.target_list_name.setCurrentIndex(0 if target_lists else -1)

            self.target_list_name.blockSignals(False)

            # Rewire handler safely and trigger once
            try:
                self.target_list_name.currentIndexChanged.disconnect()
            except TypeError:
                pass
            self.target_list_name.currentIndexChanged.connect(
                lambda *_: self.on_target_set_changed()
            )
            self.on_target_set_changed()
            self.hide_default_columns()

    def upload_new_target_list(self):
        """Handle creating a new target list, uploading CSV, and creating a new target set."""
        # remember where we were, so we can revert on cancel
        prev_idx = self.target_list_name.currentIndex()

        # 1) File dialog (single file is enough)
        file_dialog = QFileDialog(self.parent)
        file_dialog.setFileMode(QFileDialog.ExistingFile)   # was ExistingFiles
        file_dialog.setNameFilter("CSV Files (*.csv)")

        if file_dialog.exec_() == QFileDialog.Accepted:
            file_path = file_dialog.selectedFiles()[0]

            # 2) Ask for a name
            target_set_name, ok = QInputDialog.getText(self.parent, "Enter Target Set Name", "Target Set Name:")
            if ok and target_set_name:
                # Avoid cascaded signals while we clear/update the combo
                with QSignalBlocker(self.target_list_name):
                    self.target_list_name.clear()
                # 3) Do the upload (your upload already refreshes + rebuilds lists)
                self.logic_service.upload_csv_to_mysql(file_path, target_set_name)
                # (Optional) set the selection to the new name without firing handler
                with QSignalBlocker(self.target_list_name):
                    self.target_list_name.setCurrentText(target_set_name)
                self.parent.reload_table()
            else:
                # Cancelled name → put the combo back to a normal item
                with QSignalBlocker(self.target_list_name):
                    self.target_list_name.setCurrentIndex(max(0, min(prev_idx, self.target_list_name.count() - 2)))
                print("Target set creation cancelled or no name provided.")
        else:
            # Cancelled file → put the combo back to a normal item
            with QSignalBlocker(self.target_list_name):
                self.target_list_name.setCurrentIndex(max(0, min(prev_idx, self.target_list_name.count() - 2)))
            print("File selection cancelled.")


    def on_target_set_changed(self, *_):
        combo = self.target_list_name
        idx   = combo.currentIndex()
        text  = combo.currentText().strip()

        # enable Add Row only on real sets
        self.add_row_button.setEnabled(text not in ("Upload new target list", "Create empty target list", "No Target Lists Available", ""))

        last_real = max(0, combo.count() - 3)

        if text == "Upload new target list":
            with QSignalBlocker(combo):
                combo.setCurrentIndex(last_real)
            self.upload_new_target_list()
            return

        if text == "Create empty target list":
            with QSignalBlocker(combo):
                combo.setCurrentIndex(last_real)
            name, ok = QInputDialog.getText(self.parent, "Create empty target list", "Target list name:")
            if ok and name.strip():
                # create empty set; UI refreshes to show the newest (empty) set
                self.logic_service.create_empty_target_set(name.strip())
            return

        # Real selection → remember name and filter from DB
        self.parent.current_target_list_name = text
        self.logic_service.filter_target_list()
        self.hide_default_columns()

    def on_target_set_changed(self, *_):
        combo = self.target_list_name
        idx   = combo.currentIndex()
        text  = combo.currentText().strip()

        # enable Add Row only on real sets
        self.add_row_button.setEnabled(text not in ("Upload new target list", "Create empty target list", "No Target Lists Available", ""))

        last_real = max(0, combo.count() - 3)

        if text == "Upload new target list":
            with QSignalBlocker(combo):
                combo.setCurrentIndex(last_real)
            self.upload_new_target_list()
            return

        if text == "Create empty target list":
            with QSignalBlocker(combo):
                combo.setCurrentIndex(last_real)
            name, ok = QInputDialog.getText(self.parent, "Create empty target list", "Target list name:")
            if ok and name.strip():
                # create empty set; UI refreshes to show the newest (empty) set
                self.logic_service.create_empty_target_set(name.strip())
            return

        # Real selection → remember name and filter from DB
        self.parent.current_target_list_name = text
        self.logic_service.filter_target_list()
        self.hide_default_columns()

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
        etc_layout.setContentsMargins(10, 10, 10, 10)  # Add some space around the whole layout

        # Common dimensions
        uniform_input_width = 110  # Set the width of all input fields to half of the previous value
        widget_height = 35  # Set a reasonable widget height
        dropdown_width = 60  # Keep dropdown width consistent with input fields

        # Helper function to create aligned labels
        def create_aligned_label(text):
            label = QLabel(text)
            label.setFixedWidth(uniform_input_width)  # Fixed width to align with the input fields
            label.setAlignment(Qt.AlignRight | Qt.AlignVCenter)
            label.setStyleSheet("font-size: 14pt;")  # Set font size for consistency
            return label

        # Function to create a row with label and input field(s)
        def create_input_row(label_text, *widgets):
            row_layout = QHBoxLayout()
            label = create_aligned_label(label_text)
            row_layout.addWidget(label)
            for widget in widgets:
                row_layout.addWidget(widget)
                widget.setFixedHeight(widget_height)  # Uniform height for all widgets
                widget.setFixedWidth(uniform_input_width)  # Set the same width for all widgets
                widget.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)  # Prevent stretching
            return row_layout

        # Row 0: Magnitude, Filter, and System
        self.magnitude_input = QLineEdit()
        self.magnitude_input.setFixedWidth(uniform_input_width)

        self.filter_dropdown = QComboBox()
        self.filter_dropdown.addItems(["U", "G", "R", "I"])
        self.filter_dropdown.setFixedWidth(dropdown_width)

        self.system_field = QLineEdit("AB")
        self.system_field.setReadOnly(True)
        self.system_field.setFixedWidth(uniform_input_width)

        input_row_0 = create_input_row("Magnitude:", self.magnitude_input, self.filter_dropdown, self.system_field)
        etc_layout.addLayout(input_row_0)

        # Row 1: Sky Mag and SNR (labels in one line with input fields)
        self.sky_mag_input = QLineEdit()
        self.sky_mag_input.setFixedWidth(uniform_input_width)

        self.snr_input = QLineEdit()
        self.snr_input.setFixedWidth(uniform_input_width)
        self.snr_input.setFixedHeight(widget_height)

        # Add 'SNR' label next to 'Sky Mag' input field
        input_row_1 = create_input_row("Sky Mag:", self.sky_mag_input)
        input_row_1.addWidget(create_aligned_label("SNR: "))  # Add 'SNR' label next to 'Sky Mag'
        input_row_1.addWidget(self.snr_input)
        etc_layout.addLayout(input_row_1)

        # Row 2: Slit Width and Slit Width Dropdown
        self.slit_width_input = QLineEdit()
        self.slit_width_input.setFixedWidth(uniform_input_width)

        self.slit_dropdown = QComboBox()
        self.slit_dropdown.addItems(["SET", "LOSS", "SNR", "RES", "AUTO"])
        self.slit_dropdown.setFixedWidth(dropdown_width)

        input_row_2 = create_input_row("Slit Width:", self.slit_width_input, self.slit_dropdown)
        etc_layout.addLayout(input_row_2)

        # Row 3: Range and No Slicer
        self.range_input_start = QLineEdit()
        self.range_input_start.setFixedWidth(uniform_input_width)
        self.range_input_start.setFixedHeight(widget_height)

        range_dash = QLabel("-")
        range_dash.setFixedWidth(10)

        self.range_input_end = QLineEdit()
        self.range_input_end.setFixedWidth(uniform_input_width)
        self.range_input_end.setFixedHeight(widget_height)

        self.no_slicer_checkbox = QCheckBox("No Slicer")
        self.no_slicer_checkbox.setFixedHeight(widget_height)

        range_layout = QHBoxLayout()
        range_layout.setSpacing(10)
        range_layout.addWidget(create_aligned_label("Range:"))
        range_layout.addWidget(self.range_input_start)
        range_layout.addWidget(range_dash)
        range_layout.addWidget(self.range_input_end)
        range_layout.addWidget(self.no_slicer_checkbox)

        etc_layout.addLayout(range_layout)

        # Row 4: Seeing (arcsec) and Airmass
        self.seeing_input = QLineEdit()
        self.seeing_input.setFixedWidth(uniform_input_width)

        self.airmass_input = QLineEdit()
        self.airmass_input.setFixedWidth(uniform_input_width)
        self.airmass_input.setFixedHeight(widget_height)

        input_row_4 = create_input_row("Seeing:", self.seeing_input)
        input_row_4.addWidget(create_aligned_label("Airmass: "))  # Add 'SNR' label next to 'Sky Mag'
        input_row_4.addWidget(self.airmass_input)
        etc_layout.addLayout(input_row_4)

        # Row 5: EXPTIME and Resolution
        self.exptime_input = QLineEdit()
        self.exptime_input.setFixedWidth(uniform_input_width)

        self.resolution_input = QLineEdit()
        self.resolution_input.setFixedWidth(uniform_input_width)
        self.resolution_input.setFixedHeight(widget_height)

        input_row_5 = create_input_row("Exp Time:", self.exptime_input)
        input_row_5.addWidget(create_aligned_label("Resolution: "))  # Add 'SNR' label next to 'Sky Mag'
        input_row_5.addWidget(self.resolution_input)
        etc_layout.addLayout(input_row_5)

        # Divider line between the two columns
        divider_line = QFrame()
        divider_line.setFrameShape(QFrame.HLine)
        divider_line.setFrameShadow(QFrame.Sunken)

        etc_layout.addWidget(divider_line)

        # Buttons in the layout
        button_row_layout = QHBoxLayout()
        button_row_layout.setSpacing(10)

        run_button = QPushButton("Run ETC")
        run_button.setFixedSize(110, 45)
        run_button.clicked.connect(self.run_etc)

        save_button = QPushButton("Save")
        save_button.setFixedSize(100, 45)
        save_button.setEnabled(False)  # Initially disable the Save button
        save_button.clicked.connect(self.save_etc)

        button_row_layout.addWidget(run_button)
        button_row_layout.addWidget(save_button)

        # Add a spacer after the buttons to create margin below
        spacer = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding)
        button_row_layout.addItem(spacer)

        etc_layout.addLayout(button_row_layout)

        # Set the layout for the ETC tab
        self.parent.etc.setLayout(etc_layout)

        # Add a spacer to ensure widgets aren't squished
        spacer = QSpacerItem(20, 30, QSizePolicy.Minimum, QSizePolicy.Expanding)
        etc_layout.addItem(spacer)

    def create_target_dropdown_group(self):
        # Create the group box for Target List
        target_dropdown_group = QGroupBox("Target List")
        target_dropdown_layout = QVBoxLayout()
        target_dropdown_layout.setSpacing(10)

        # Create the toggle button
        self.target_list_mode_toggle = QPushButton("Mode: Science")
        self.target_list_mode_toggle.setCheckable(True)
        self.target_list_mode_toggle.setMaximumWidth(200)

        # Set toggle styles (optional)
        self.target_list_mode_toggle.setStyleSheet("""
            QPushButton {
                background-color: #4CAF50;
                color: white;
                font-weight: bold;
                border-radius: 6px;
                padding: 4px;
            }
            QPushButton:checked {
                background-color: #2196F3;
            }
        """)

        # Toggle between "Science" and "Calibration"
        self.target_list_mode_toggle.toggled.connect(self.on_toggle_target_mode)

        # Add the toggle button to the layout
        target_dropdown_layout.addWidget(self.target_list_mode_toggle)

        # Add the target list name combo box
        self.target_list_name = QComboBox()
        self.target_list_name.setMaximumWidth(250)
        target_dropdown_layout.addWidget(self.target_list_name)

        # Load the initial target lists
        self.load_target_lists()

        target_dropdown_group.setLayout(target_dropdown_layout)
        target_dropdown_group.setMaximumWidth(300)
        target_dropdown_group.setMaximumHeight(150)

        return target_dropdown_group

    def on_toggle_target_mode(self, checked):
        # Update button text
        if checked:
            self.target_list_mode_toggle.setText("Mode: Calibration")
        else:
            self.target_list_mode_toggle.setText("Mode: Science")

        # Reload target lists based on the new mode
        self.load_target_lists()

    def update_lamps(self, lamp_states):
        """Update the lamp checkboxes based on the received state."""
        
        # Mapping of incoming lamp names to checkbox names
        lamp_name_map = {
            'LAMPBLUC': 'BlueCont',
            'LAMPFEAR': 'FeAr',
            'LAMPREDC': 'RedCont',
            'LAMPTHAR': 'ThAR'
        }

        for lamp, state in lamp_states.items():
            mapped_lamp = lamp_name_map.get(lamp)
            
            if mapped_lamp:
                checkbox = self.lamp_checkboxes.get(mapped_lamp)
                
                if checkbox:
                    checkbox.setChecked(state)
                    checkbox.setText("On" if state else "Off") 

                    # Optional: apply a visual style for on/off
                    if state:
                        checkbox.setStyleSheet("""
                            QCheckBox::indicator:checked {
                                background-color: green;
                                border: 2px solid darkgreen;
                            }
                        """)
                    else:
                        checkbox.setStyleSheet("""
                            QCheckBox::indicator:unchecked {
                                background-color: gray;
                                border: 2px solid darkgray;
                            }
                        """)
                else:
                    print(f"Checkbox for {mapped_lamp} not found")
            else:
                print(f"Lamp {lamp} not mapped to a checkbox")

    def update_modulators(self, modulator_states):
        """Update the modulator checkboxes based on the received state."""

        # Mapping of incoming modulator names to checkbox names
        modulator_name_map = {
            'MODTHAR': 'ThAR',
            'MODFEAR': 'FeAr',
            'MODRDCON': 'RedCont',
            'MODBLCON': 'BlueCont'
        }

        for modulator, state in modulator_states.items():
            mapped_modulator = modulator_name_map.get(modulator)

            if mapped_modulator:
                checkbox = self.modulator_checkboxes.get(mapped_modulator)

                if checkbox:
                    print(f"Setting {mapped_modulator} checkbox to {state}")
                    checkbox.setChecked(state)
                    checkbox.setText("On" if state else "Off")

                    # Style the checkbox based on whether it's checked or not
                    if state:
                        checkbox.setStyleSheet("""
                            QCheckBox::indicator:checked {
                                background-color: green;
                                border: 2px solid darkgreen;
                            }
                        """)
                    else:
                        checkbox.setStyleSheet("""
                            QCheckBox::indicator:unchecked {
                                background-color: gray;
                                border: 2px solid darkgray;
                            }
                        """)
                else:
                    print(f"Checkbox for {mapped_modulator} not found")
            else:
                print(f"Modulator {modulator} not mapped to a checkbox")

