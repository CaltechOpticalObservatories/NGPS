import mysql.connector
import configparser
from PyQt5.QtWidgets import QTableWidgetItem
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor, QFont
import os
import csv
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

    def read_config(self, config_file):
        """
        Reads configuration from the provided file.
        This function should load and return the configuration (host, user, password, etc.)
        """
        # Assuming a simple INI format; you'd implement the actual reading logic here
        # This is a simplified version for demonstration
        config = {}
        try:
            with open(config_file, "r") as f:
                for line in f:
                    if line.strip() and not line.startswith(";"):
                        key, value = line.split("=")
                        config[key.strip()] = value.strip()
        except Exception as e:
            print(f"Error reading config file: {e}")
        return config

    def connect_to_mysql(self, config_file):
        """
        Connect to MySQL using the configuration file and return the connection object.
        Logs connection and error details.
        """
        db_config = self.read_config(config_file)
        
        # Log the connection attempt
        print("Attempting to connect to MySQL database...")
        
        try:
            # Connect to MySQL without selecting a database
            connection = mysql.connector.connect(
                host=db_config["SYSTEM"],  # Hostname from the config file
                user=db_config["USERNAME"],  # MySQL user from the config file
                password=db_config["PASSWORD"],  # Password from the config file
            )

            # Log that the connection to the MySQL server was successful
            print(f"Successfully connected to MySQL server: {db_config['SYSTEM']}.")

            # Select the database after establishing the connection
            cursor = connection.cursor()
            cursor.execute(f"USE {db_config['DBMS']};")  # Ensure we are using the correct database
            print(f"Successfully selected database: {db_config['DBMS']}.")

            # Optionally, you can verify the database selection with the following:
            cursor.execute("SELECT DATABASE();")
            current_db = cursor.fetchone()[0]
            print(f"Currently connected to database: {current_db}")

            cursor.close()

            # Return the connection object after ensuring the correct database is selected
            return connection
        
        except mysql.connector.Error as err:
            # If an error occurs, log the error and return None
            print(f"Error connecting to MySQL: {err}")
            return None


    def load_data_from_mysql(self, connection, target_table):
        """
        Loads target data from MySQL using the provided connection.
        Returns the fetched rows from the target table.
        """
        try:
            cursor = connection.cursor(dictionary=True)  # Use dictionary for column name access
            
            # Query the target data from the database (modify as needed)
            cursor.execute(f"SELECT * FROM {target_table}")
            rows = cursor.fetchall()  # Fetch all rows
            
            cursor.close()
            return rows
        except mysql.connector.Error as err:
            print(f"Error executing query: {err}")
            return []

    def load_mysql_and_update_target_list(self, config_file):
        """
        Loads target data from MySQL and updates the target list table.
        This method is now a high-level function that combines both connecting to MySQL and loading data.
        """
        # Step 1: Connect to MySQL using the config file
        connection = self.connect_to_mysql(config_file)
        
        if connection is None:
            print("Failed to connect to MySQL. Cannot load target data.")
            return
        
        # Step 2: Load data from the MySQL database
        db_config = self.read_config(config_file)  # We need to read config again for the table name
        target_table = db_config["TARGET_TABLE"]
        rows = self.load_data_from_mysql(connection, target_table)
        
        if rows:
            # Step 3: Update the table with the data
            self.update_target_list_table(rows)
        else:
            print(f"No data found in the {target_table} table.")
        
        # Close the database connection after usage
        connection.close()

    def update_target_list_table(self, data):
        """
        Populates the UI table with data from the MySQL database.
        The columns and rows will be dynamically created based on the data.
        It hides the specified columns.
        """
        target_list_display = self.parent.layout_service.target_list_display

        # Step 1: Clear existing rows in the target list
        target_list_display.setRowCount(0)

        # List of columns to hide
        columns_to_hide = [
            "OBSERVATION_ID", "SET_ID", "STATE", "OBS_ORDER", "TARGET_NUMBER",
            "SEQUENCE_NUMBER", "OTMexpt", "OTMslitwidth", "OTMcass", "OTMairmass_start",
            "OTMairmass_end", "OTMsky", "OTMdead", "OTMslewgo", "OTMexp_start", "OTMexp_end",
            "OTMpa", "OTMwait", "OTMflag", "OTMlast", "OTMslew", "OTMmoon", "OTMSNR",
            "OTMres", "OTMseeing", "OTMslitangle", "NOTE"
        ]

        # Step 2: Filter out unwanted columns and their data
        filtered_data = []
        filtered_column_names = []

        for row_data in data:
            # Create a new row data dictionary that excludes the unwanted columns
            filtered_row = {key: value for key, value in row_data.items() if key not in columns_to_hide}
            filtered_data.append(filtered_row)

        # Step 3: Set the number of columns dynamically based on the filtered data
        if filtered_data:
            # Extract the column names from the first row (assuming all rows have the same structure)
            filtered_column_names = filtered_data[0].keys()

            # Set the column count
            target_list_display.setColumnCount(len(filtered_column_names))

            # Set the header labels based on the filtered column names
            target_list_display.setHorizontalHeaderLabels(filtered_column_names)

            # Remove the bold font from headers
            header = target_list_display.horizontalHeader()
            header.setFont(QFont("Arial", 10, QFont.Normal))  # Set font to normal (non-bold)

            # Step 4: Add new rows based on the filtered data
            for row_data in filtered_data:
                row_position = target_list_display.rowCount()
                target_list_display.insertRow(row_position)

                # Dynamically populate the table with the filtered data
                for col_index, (col_name, value) in enumerate(row_data.items()):
                    target_list_display.setItem(row_position, col_index, QTableWidgetItem(str(value)))

            # Step 5: Optionally, sort the table if you want to auto-sort after loading
            target_list_display.sortItems(0, Qt.AscendingOrder)  # Example: sort by first column (name)

            # Step 6: Hide the button and show the table once the data is loaded
            self.parent.layout_service.load_target_button.setVisible(False)  # Hide the load button
            target_list_display.setVisible(True)  # Show the table


    def update_target_information(self, target_data):
        # Pass the dictionary of target data to LayoutService to update the list
        self.parent.layout_service.no_target_label.setVisible(False)
        self.parent.layout_service.update_target_info_form(target_data)
