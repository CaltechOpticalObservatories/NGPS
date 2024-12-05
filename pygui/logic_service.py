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
        self.connection = None
        self.all_targets = []

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
            self.connection = mysql.connector.connect(
                host=db_config["SYSTEM"],  # Hostname from the config file
                user=db_config["USERNAME"],  # MySQL user from the config file
                password=db_config["PASSWORD"],  # Password from the config file
            )

            # Log that the connection to the MySQL server was successful
            print(f"Successfully connected to MySQL server: {db_config['SYSTEM']}.")

            # Select the database after establishing the connection
            cursor = self.connection.cursor()
            cursor.execute(f"USE {db_config['DBMS']};")  # Ensure we are using the correct database
            print(f"Successfully selected database: {db_config['DBMS']}.")

            # Optionally, you can verify the database selection with the following:
            cursor.execute("SELECT DATABASE();")
            current_db = cursor.fetchone()[0]
            print(f"Currently connected to database: {current_db}")

            cursor.close()
            
            # Return the connection object after ensuring the correct database is selected
            return self.connection

        except mysql.connector.Error as err:
            # If an error occurs, log the error and set connection to None
            print(f"Error connecting to MySQL: {err}")
            self.connection = None

    def close_connection(self):
        """
        Closes the current MySQL connection if it exists.
        """
        if self.connection:
            self.connection.close()
            print("MySQL connection closed.")
        self.connection = None


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
        
        # Step 2: Load data from the MySQL database (target_table)
        db_config = self.read_config(config_file)  # We need to read config again for the table name
        target_table = db_config["TARGET_TABLE"]
        rows = self.load_data_from_mysql(connection, target_table)
        
        if rows:
            # Step 3: Update the target list table with the data
            self.update_target_list_table(rows)

            # Populate the target_list_name dropdown with available target lists (e.g., based on SET_ID)
            target_list_names = sorted(set(row['SET_ID'] for row in rows))  # Unique SET_IDs or Target List names
            self.parent.target_list_name.clear()  # Clear existing items
            self.parent.target_list_name.addItem("All")  # Option to select all target lists
            self.parent.target_list_name.addItems([str(name) for name in target_list_names])  # Add each SET_ID to the dropdown

            # Connect the combo box selection change to filtering function
            self.parent.target_list_name.currentIndexChanged.connect(self.filter_target_list)
        else:
            print(f"No data found in the {target_table} table.")
        
        # Close the database connection after usage
        connection.close()


    def load_mysql_and_fetch_target_sets(self, config_file):
        """
        Loads target set data from the 'target_sets' table and performs actions to update the UI.
        This method combines both connecting to MySQL and loading data from the 'target_sets' table.
        """
        # Step 1: Connect to MySQL using the config file
        connection = self.connect_to_mysql(config_file)
        
        if connection is None:
            print("Failed to connect to MySQL. Cannot load target set data.")
            return
        
        # Step 2: Load data from the 'target_sets' table
        db_config = self.read_config(config_file)  # We need to read config again for the table name
        target_sets_table = db_config["TARGET_SETS_TABLE"]  # Assuming you have the 'target_sets' table name in config
        rows = self.load_data_from_mysql(connection, target_sets_table)
        
        if rows:
            # Step 3: Handle the data (update the UI, or pass it to another function)
            print(f"Fetched {len(rows)} target sets.")
            # You can process the rows as needed, e.g., updating a table in the UI
            self.update_target_sets_table(rows)
        else:
            print(f"No data found in the {target_sets_table} table.")
        
        # Close the database connection after usage
        connection.close()

    def filter_target_list(self):
        """
        Filters the target list table based on the selected SET_ID from the target_list_name combo box.
        If "All" is selected, it shows all targets.
        """
        selected_set_id = self.parent.target_list_name.currentText()
        
        # If the user selects "All", display all targets
        if selected_set_id == "All":
            self.update_target_list_table(self.all_target_data)  # Assuming all_target_data holds all rows
        else:
            # Filter the rows based on the selected SET_ID
            filtered_data = [row for row in self.all_target_data if str(row['SET_ID']) == selected_set_id]
            self.update_target_list_table(filtered_data)

    def update_target_list_table(self, all_targets, target_list_name):
        """
        Populates the UI table with data for the selected target set.
        Dynamically updates the table based on the selected target set.
        It hides the specified columns.
        
        :param all_targets: Dictionary where keys are SET_ID and values are a dict with SET_NAME and targets.
        :param target_list_name: List of SET_NAMEs to display in the dropdown for selection.
        """
        target_list_display = self.parent.layout_service.target_list_display
        self.all_targets = all_targets

        # Step 1: Clear existing rows in the target list
        target_list_display.setRowCount(0)

        # List of columns to hide
        columns_to_hide = [
            "SET_ID", "STATE", "OBS_ORDER", "TARGET_NUMBER",
            "SEQUENCE_NUMBER", "OTMexpt", "OTMslitwidth", "OTMcass", "OTMairmass_start",
            "OTMairmass_end", "OTMsky", "OTMdead", "OTMslewgo", "OTMexp_start", "OTMexp_end",
            "OTMpa", "OTMwait", "OTMflag", "OTMlast", "OTMslew", "OTMmoon", "OTMSNR",
            "OTMres", "OTMseeing", "OTMslitangle", "NOTE", "OWNER", "NOTBEFORE", "POINTMODE"
        ]

        # Step 2: Check if the selected target set exists in the provided all_targets
        for selected_set_name in target_list_name:
            if selected_set_name in self.all_targets:
                set_info = self.all_targets[selected_set_name]
                data = set_info["targets"]

                # Step 3: Filter out unwanted columns and their data
                filtered_data = []
                filtered_column_names = []

                for row_data in data:
                    # Create a new row data dictionary that excludes the unwanted columns
                    filtered_row = {key: value for key, value in row_data.items() if key not in columns_to_hide}
                    filtered_data.append(filtered_row)

                # Step 4: Set the number of columns dynamically based on the filtered data
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

                    # Step 5: Add new rows based on the filtered data
                    for row_data in filtered_data:
                        row_position = target_list_display.rowCount()
                        target_list_display.insertRow(row_position)

                        # Dynamically populate the table with the filtered data
                        for col_index, (col_name, value) in enumerate(row_data.items()):
                            target_list_display.setItem(row_position, col_index, QTableWidgetItem(str(value)))

                    # Step 6: Optionally, sort the table if you want to auto-sort after loading
                    target_list_display.sortItems(0, Qt.AscendingOrder)  # Example: sort by first column (name)

                    # Step 7: Hide the button and show the table once the data is loaded
                    self.parent.layout_service.load_target_button.setVisible(False)  # Hide the load button
                    target_list_display.setVisible(True)  # Show the table
            else:
                print(f"Error: Target set {selected_set_name} not found in all_targets data.")



    def update_target_information(self, target_data):
        # Pass the dictionary of target data to LayoutService to update the list
        self.parent.layout_service.no_target_label.setVisible(False)
        self.parent.layout_service.update_target_info_form(target_data)

    def send_update_to_db(self, observation_id, field_name, value):
        """
        Sends an update query to the database to modify a specific field for the given observation ID.
        """
        try:
            # Step 1: Connect to MySQL using the config file
            connection = self.connect_to_mysql("config/db_config.ini")
            
            if connection is None:
                print("Failed to connect to MySQL. Cannot load target data.")
                return
            cursor = connection.cursor()  # Create a cursor for executing the query

            # Prepare the SQL query to update the field in the database
            query = f"UPDATE targets SET {field_name} = %s WHERE observation_id = %s"

            # Execute the query with the provided value and observation_id
            cursor.execute(query, (value, observation_id))

            # Commit the transaction to apply the changes
            connection.commit()

            cursor.close()
            print(f"Successfully updated {field_name} to {value} for observation ID {observation_id}")
        except mysql.connector.Error as err:
            print(f"Error executing update query: {err}")
            
    def refresh_table(self):
        """
        Refreshes the table by querying the database for the latest data
        and updating the QTableWidget with the new data.
        """
        try:
            # Example query to fetch the latest target data
            connection = self.connect_to_mysql("config/db_config.ini")  # Assuming you have a method to connect to the DB

            if connection is None:
                print("Failed to connect to MySQL. Cannot refresh table.")
                return

            cursor = connection.cursor()

            # Fetch all the latest data from the database (e.g., all target data)
            cursor.execute("SELECT observation_id, name, exptime, slitwidth FROM target")  # Adjust query as needed
            rows = cursor.fetchall()
            
            target_list_display = self.parent.layout_service.target_list_display
            # Clear existing table data
            target_list_display.setRowCount(0)  # Reset row count

            # Add new data to the table
            for row in rows:
                row_position = target_list_display.rowCount()
                target_list_display.insertRow(row_position)
                for column, value in enumerate(row):
                    target_list_display.setItem(row_position, column, QTableWidgetItem(str(value)))

            cursor.close()
            print("Table refreshed successfully!")

        except mysql.connector.Error as err:
            print(f"Error while refreshing table: {err}")