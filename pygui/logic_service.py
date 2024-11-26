import csv
from PyQt5.QtWidgets import QTableWidgetItem
from PyQt5.QtCore import Qt
import pytz

class LogicService:
    def __init__(self, parent):
        self.parent = parent  # reference to the parent window or main UI

    @staticmethod
    def convert_pst_to_utc(datetime):
        # Convert from PST to UTC
        timezone_pst = pytz.timezone('US/Pacific')
        timezone_utc = pytz.utc
        start_time_pst = datetime.toPyDateTime()
        start_time_pst = timezone_pst.localize(start_time_pst)
        start_time_utc = start_time_pst.astimezone(timezone_utc)
        return start_time_utc

    def load_csv_and_update_target_list(self, file_path):
        # Step 1: Read CSV file
        try:
            with open(file_path, 'r') as file:
                reader = csv.DictReader(file)  # Read CSV into a dictionary
                data = list(reader)  # Convert to a list of rows
                
                # Step 2: Update the table with the CSV data
                self.update_target_list_table(data)
        
        except Exception as e:
            print(f"Error loading CSV file: {e}")

    def update_target_list_table(self, data):
        # Access the table from LayoutService (parent layout)
        target_list_display = self.parent.layout_service.target_list_display
        
        # Step 3: Clear existing rows in the target list
        target_list_display.setRowCount(0)
        
        # Step 4: Add new rows based on CSV data
        for row_data in data:
            row_position = target_list_display.rowCount()
            target_list_display.insertRow(row_position)
            
            # Step 5: Populate the table with CSV data (ensure columns match)
            target_list_display.setItem(row_position, 0, QTableWidgetItem(row_data["name"]))
            target_list_display.setItem(row_position, 1, QTableWidgetItem(row_data["RA"]))
            target_list_display.setItem(row_position, 2, QTableWidgetItem(row_data["DECL"]))
            target_list_display.setItem(row_position, 3, QTableWidgetItem(row_data["binspect"]))
            target_list_display.setItem(row_position, 4, QTableWidgetItem(row_data["binspat"]))
            target_list_display.setItem(row_position, 5, QTableWidgetItem(row_data["ccdmode"]))
            target_list_display.setItem(row_position, 6, QTableWidgetItem(row_data["slitangle"]))
            target_list_display.setItem(row_position, 7, QTableWidgetItem(row_data["slitwidth"]))
            target_list_display.setItem(row_position, 8, QTableWidgetItem(row_data["exptime"]))
            target_list_display.setItem(row_position, 9, QTableWidgetItem(row_data["airmass_max"]))
            
            # Add any other fields as necessary...

        # Step 6: Optionally, sort the table if you want to auto-sort after loading
        target_list_display.sortItems(0, Qt.AscendingOrder)  # Example: sort by first column (name)

        # Step 7: Hide the button and show the table once the CSV is loaded
        self.parent.layout_service.load_target_button.setVisible(False)  # Hide the load button
        target_list_display.setVisible(True)  # Show the table


    def update_target_information(self, target_data):
        # Pass the dictionary of target data to LayoutService to update the list
        self.parent.layout_service.no_target_label.setVisible(False)
        self.parent.layout_service.update_target_info_form(target_data)