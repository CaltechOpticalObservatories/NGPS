import mysql.connector
import configparser
from PyQt5.QtWidgets import QTableWidgetItem
from PyQt5.QtCore import Qt, pyqtSignal
from PyQt5.QtGui import QColor, QFont
import os
import csv
import pytz
from astropy.coordinates import SkyCoord, EarthLocation
from astropy.time import Time
from astroplan import Observer
import astropy.units as u
import datetime

class LogicService:
    def __init__(self, parent):
        self.parent = parent  # reference to the parent window or main UI
        self.connection = None
        self.all_targets = []
        self.target_list_set = {}
        self.target_list_display = None

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

    def upload_csv_to_mysql(self, file_path, target_set_name):
        """Upload the CSV to MySQL and associate it with a new target set."""
        
        # Step 1: Read the CSV
        try:
            with open(file_path, 'r') as file:
                reader = csv.DictReader(file)
                data = list(reader)  # Convert CSV to list of dictionaries
                print(f"CSV file loaded. Total rows: {len(data)}")
        except Exception as e:
            print(f"Error reading CSV file: {e}")
            return

        # Step 2: Insert a new entry into the `target_sets` table
        try:
            cursor = self.connection.cursor()
            cursor.execute("INSERT INTO target_sets (SET_NAME, OWNER, SET_CREATION_TIMESTAMP) VALUES (%s, %s, NOW())",
                        (target_set_name, self.parent.current_owner))  # Use the current owner's info
            self.connection.commit()

            # Step 3: Fetch the new SET_ID (for the newly inserted target set)
            cursor.execute("SELECT LAST_INSERT_ID()")
            set_id = cursor.fetchone()[0]
            print(f"New target set created with SET_ID: {set_id}")
            cursor.close()

            # Step 4: Dynamically construct the insert query
            cursor = self.connection.cursor()

            # Get the columns in the CSV file (i.e., DictReader's fieldnames)
            csv_columns = reader.fieldnames  # List of column names from the CSV
            print(f"CSV Columns: {csv_columns}")

            # Define all columns from your table `targets` (based on your schema)
            all_columns = [
                'OBSERVATION_ID', 'STATE', 'OBS_ORDER', 'TARGET_NUMBER', 'SEQUENCE_NUMBER', 'NAME', 
                'RA', 'DECL', 'OFFSET_RA', 'OFFSET_DEC', 'EXPTIME', 'SLITWIDTH', 'SLITOFFSET', 'OBSMODE', 
                'BINSPECT', 'BINSPAT', 'SLITANGLE', 'AIRMASS_MAX', 'WRANGE_LOW', 'WRANGE_HIGH', 'CHANNEL', 
                'MAGNITUDE', 'MAGSYSTEM', 'MAGFILTER', 'SRCMODEL', 'OTMexpt', 'OTMslitwidth', 'OTMcass', 
                'OTMairmass_start', 'OTMairmass_end', 'OTMsky', 'OTMdead', 'OTMslewgo', 'OTMexp_start', 
                'OTMexp_end', 'OTMpa', 'OTMwait', 'OTMflag', 'OTMlast', 'OTMslew', 'OTMmoon', 'OTMSNR', 
                'OTMres', 'OTMseeing', 'OTMslitangle', 'NOTE', 'COMMENT', 'OWNER', 'NOTBEFORE', 'POINTMODE'
            ]

            # Step 5: Loop through each row and dynamically generate the insert query
            for idx, row in enumerate(data):
                row_data = [set_id]  # Start with the SET_ID as the first element in row_data
                insert_columns = ['SET_ID']  # Always include SET_ID
                insert_placeholders = ['%s']  # Placeholder for SET_ID

                print(f"Inserting row {idx + 1}: {row}")

                # Step 6: Add only non-empty fields to the insert query
                for column in all_columns:
                    value = row.get(column)  # Get the value from the CSV, default to None if not present

                    # Handle missing columns (apply defaults based on column type)
                    if value is None:
                        # Numeric columns (defaults to 0)
                        if column in ['OBS_ORDER', 'TARGET_NUMBER', 'SEQUENCE_NUMBER', 'BINSPECT', 'BINSPAT']:
                            value = 0
                        # Text columns (defaults to empty string "")
                        elif column in ['STATE', 'RA', 'DECL', 'EXPTIME', 'SLITWIDTH']:
                            value = ""  # Empty string as default for text fields
                        # # Timestamp columns (defaults to NULL)
                        # elif column in ['NOTBEFORE', 'OTMslewgo', 'OTMexp_start', 'OTMexp_end']:
                        #     value = None  # Default to NULL for timestamps
                        # else:
                        #     value = None  # Default to NULL for other columns without a defined default

                    # Special case: if `OFFSET_RA` or `OFFSET_DEC` are empty, set them to 0.0
                    if column in ['OFFSET_RA', 'OFFSET_DEC'] and (value == '' or value is None):
                        value = 0.0  # Default to 0.0 if empty or None

                    # Add the column and value to the query
                    insert_columns.append(column)
                    insert_placeholders.append('%s')
                    row_data.append(value)

                # Build the dynamic insert query
                insert_columns_str = ", ".join(insert_columns)
                insert_placeholders_str = ", ".join(insert_placeholders)
                insert_query = f"""
                    INSERT INTO targets ({insert_columns_str})
                    VALUES ({insert_placeholders_str})
                """
                
                print(f"Executing query: {insert_query}")
                print(f"With data: {row_data}")

                # Execute the insert query with dynamically generated values
                cursor.execute(insert_query, row_data)

            # Commit the transaction
            self.connection.commit()
            cursor.close()

            print(f"Successfully uploaded {len(data)} targets to the new set {target_set_name}.")
            # Emit the signal after the upload is complete
            self.load_mysql_and_fetch_target_sets()
            self.update_target_table_with_list(target_list=target_set_name)

        except mysql.connector.Error as err:
            print(f"Error inserting data into MySQL: {err}")

    def get_or_create_target_set(self, connection, target_set_name):
        """
        Checks if a target set with the given name exists. If it exists, returns the existing `SET_ID`.
        If it doesn't exist, creates a new target set and returns the new `SET_ID`.
        """
        try:
            cursor = connection.cursor()

            # Check if the target set already exists by name
            query = "SELECT SET_ID FROM target_sets WHERE SET_NAME = %s"
            cursor.execute(query, (target_set_name,))
            result = cursor.fetchone()

            if result:
                # If the target set exists, return the existing SET_ID
                set_id = result[0]
                print(f"Found existing target set with SET_ID: {set_id}")
            else:
                # If not, create a new target set and get the new SET_ID
                set_id = self.create_target_set(connection, target_set_name)
            
            cursor.close()
            return set_id
        
        except mysql.connector.Error as err:
            print(f"Error checking or creating target set: {err}")
            return None

    def create_target_set(self, connection, target_set_name):
        """
        Creates a new entry in the `target_sets` table and returns the new `SET_ID`.
        """
        try:
            cursor = connection.cursor()

            # Get the current timestamp for the new target set creation
            current_timestamp = pytz.utc.localize(datetime.datetime.utcnow()).strftime('%Y-%m-%d %H:%M:%S')

            # Insert the new target set into the `target_sets` table
            query = """
                INSERT INTO target_sets (SET_NAME, SET_CREATION_TIMESTAMP)
                VALUES (%s, %s)
            """
            cursor.execute(query, (target_set_name, current_timestamp))

            # Commit the transaction to make sure the SET_ID is generated
            connection.commit()

            # Get the newly created SET_ID
            cursor.execute("SELECT LAST_INSERT_ID();")
            set_id = cursor.fetchone()[0]

            cursor.close()

            print(f"Created new target set with SET_ID: {set_id}")
            return set_id
        
        except mysql.connector.Error as err:
            print(f"Error creating target set: {err}")
            return None

    def insert_targets_into_mysql(self, connection, data, set_id):
        """
        Inserts the data into the `targets` table, linking each target to the given `SET_ID`.
        """
        try:
            cursor = connection.cursor()
            
            # Insert each target in the CSV into the `targets` table, linking it to the `SET_ID`
            for row in data:
                # Add the SET_ID to the row data before inserting
                row['SET_ID'] = set_id
                
                # Prepare the SQL query for insertion: matching columns
                columns = ', '.join(row.keys())  # Column names from CSV (and SET_ID)
                values = ', '.join(['%s'] * len(row))  # Placeholder for values
                query = f"INSERT INTO targets ({columns}) VALUES ({values})"
                
                # Execute the insert query
                cursor.execute(query, tuple(row.values()))
            
            # Commit the transaction
            connection.commit()
            print(f"Successfully uploaded {len(data)} rows to the 'targets' table.")
        
        except mysql.connector.Error as err:
            print(f"Error inserting data into MySQL: {err}")
        
        finally:
            cursor.close()

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
            print("SET IDSSSS: ", target_list_names)
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

    def update_target_list_table(self, data):
        """
        Populates the UI table with data from the MySQL database.
        The columns and rows will be dynamically created based on the data.
        It hides the specified columns.s
        """

        self.target_list_display = self.parent.layout_service.target_list_display
        self.all_targets = data

        # Step 1: Clear existing rows in the target list
        self.target_list_display.setRowCount(0)

        # List of columns to hide
        columns_to_hide = [
            "SET_ID", "STATE", "OBS_ORDER", "TARGET_NUMBER",
            "SEQUENCE_NUMBER", "SLITOFFSET", "OBSMODE", "AIRMASS_MAX", "WRANGE_LOW",  "WRANGE_HIGH", "OTMexpt", "OTMslitwidth", "OTMcass", "OTMairmass_start",
            "OTMairmass_end", "OTMsky", "OTMdead", "OTMslewgo", "OTMexp_start", "OTMexp_end",
            "OTMpa", "OTMwait", "OTMflag", "OTMlast", "OTMslew", "OTMmoon", "OTMSNR",
            "OTMres", "OTMseeing", "OTMslitangle", "NOTE", "OWNER", "NOTBEFORE", "POINTMODE"
        ]

        # Step 2: Check if the data is a list (entire dataset) or a single row (selected row)
        if isinstance(data, list):  # If data is a list (entire dataset)
            # Filter out unwanted columns and their data
            filtered_data = []
            for row_data in data:
                # Create a new row data dictionary that excludes the unwanted columns
                filtered_row = {key: value for key, value in row_data.items() if key not in columns_to_hide}
                filtered_data.append(filtered_row)

            # Step 3: Set the number of columns dynamically based on the filtered data
            if filtered_data:
                # Extract the column names from the first row (assuming all rows have the same structure)
                filtered_column_names = filtered_data[0].keys()

                # Set the column count
                self.target_list_display.setColumnCount(len(filtered_column_names))

                # Set the header labels based on the filtered column names
                self.target_list_display.setHorizontalHeaderLabels(filtered_column_names)

                # Remove the bold font from headers
                header = self.target_list_display.horizontalHeader()
                header.setFont(QFont("Arial", 10, QFont.Normal))  # Set font to normal (non-bold)

                # Step 4: Add new rows based on the filtered data
                for row_data in filtered_data:
                    row_position = self.target_list_display.rowCount()
                    self.target_list_display.insertRow(row_position)

                    # Dynamically populate the table with the filtered data
                    for col_index, (col_name, value) in enumerate(row_data.items()):
                        self.target_list_display.setItem(row_position, col_index, QTableWidgetItem(str(value)))

                # Step 5: Optionally, sort the table if you want to auto-sort after loading
                self.target_list_display.sortItems(0, Qt.AscendingOrder)  # Example: sort by first column (name)

        else:  # If data is a single row (selected row)
            # Create a new row data dictionary that excludes the unwanted columns
            filtered_row = {key: value for key, value in data.items() if key not in columns_to_hide}
            filtered_data = [filtered_row]  # Treat the single row as a list for consistency

            # Step 3: Set the number of columns dynamically based on the filtered data
            if filtered_data:
                filtered_column_names = filtered_data[0].keys()

                # Set the column count
                self.target_list_display.setColumnCount(len(filtered_column_names))

                # Set the header labels based on the filtered column names
                self.target_list_display.setHorizontalHeaderLabels(filtered_column_names)

                # Remove the bold font from headers
                header = self.target_list_display.horizontalHeader()
                header.setFont(QFont("Arial", 10, QFont.Normal))  # Set font to normal (non-bold)

                # Step 4: Add the row based on the filtered data
                row_data = filtered_data[0]
                row_position = self.target_list_display.rowCount()
                self.target_list_display.insertRow(row_position)

                # Dynamically populate the table with the filtered data
                for col_index, (col_name, value) in enumerate(row_data.items()):
                    self.target_list_display.setItem(row_position, col_index, QTableWidgetItem(str(value)))

            # No need to sort the table since it's just one row

        # Step 6: Optionally, hide the button and show the table once the data is loaded
        self.parent.layout_service.load_target_button.setVisible(False)  # Hide the load button
        self.target_list_display.setVisible(True)  # Show the table
        self.parent.layout_service.set_column_widths()

    def update_target_table_with_list(self, target_list=None):
        """
        Populates the UI table with data from the MySQL database.
        The columns and rows will be dynamically created based on the data.
        It hides the specified columns.
        """
        self.target_list_display = self.parent.layout_service.target_list_display

        # Step 1: Find the set_id that corresponds to the target_list
        set_id = None
        for key, val in self.parent.user_set_data.items():
            if val == target_list:
                set_id = key
                break  # Stop once we've found the matching set_id

        if set_id is None:
            print("set_id not found for the given target_list")
            return  # Exit if no matching set_id is found

        # List of columns to hide
        columns_to_hide = [
            "SET_ID", "STATE", "OBS_ORDER", "TARGET_NUMBER",
            "SEQUENCE_NUMBER", "OTMexpt", "OTMslitwidth", "OTMcass", "OTMairmass_start",
            "OTMairmass_end", "OTMsky", "OTMdead", "OTMslewgo", "OTMexp_start", "OTMexp_end",
            "OTMpa", "OTMwait", "OTMflag", "OTMlast", "OTMslew", "OTMmoon", "OTMSNR",
            "OTMres", "OTMseeing", "OTMslitangle", "NOTE", "OWNER", "NOTBEFORE", "POINTMODE"
        ]

        # Step 2: Filter the data by set_id (SET_ID column)
        filtered_data = []
        if isinstance(self.parent.all_targets, list):  # If data is a list (entire dataset)
            for row_data in self.parent.all_targets:
                # Filter rows where 'SET_ID' matches the set_id
                if row_data.get('SET_ID') == set_id:
                    # Create a new row data dictionary that excludes the unwanted columns
                    filtered_row = {key: value for key, value in row_data.items() if key not in columns_to_hide}
                    filtered_data.append(filtered_row)

        else:  # If data is a single row (selected row)
            row_data = self.parent.all_targets
            if row_data.get('SET_ID') == set_id:
                # Filter the single row
                filtered_row = {key: value for key, value in row_data.items() if key not in columns_to_hide}
                filtered_data = [filtered_row]  # Treat the single row as a list for consistency

        # Step 3: Clear the existing content of the QTableWidget
        self.target_list_display.setRowCount(0)  # Clear all items in the table

        # Step 4: Dynamically create the table based on filtered data
        if filtered_data:
            # Extract column names from the first row
            filtered_column_names = list(filtered_data[0].keys())
            # Set the column count first to avoid any mismatch
            self.target_list_display.setColumnCount(len(filtered_column_names))
            self.target_list_display.setHorizontalHeaderLabels(filtered_column_names)

            # Add rows
            for row_data in filtered_data:
                row_position = self.target_list_display.rowCount()
                self.target_list_display.insertRow(row_position)

                # Insert data into columns
                for col_index, (col_name, value) in enumerate(row_data.items()):
                    item = QTableWidgetItem(str(value))
                    self.target_list_display.setItem(row_position, col_index, item)

            # Step 6: Optionally, sort the table if you want to auto-sort after loading
            self.target_list_display.sortItems(0, Qt.AscendingOrder)  # Example: sort by first column (name)

        # Step 7: Optionally, hide the button and show the table once the data is loaded
        self.parent.layout_service.load_target_button.setVisible(False)  # Hide the load button
        self.target_list_display.setVisible(True)  # Show the table


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
            
    def compute_parallactic_angle_astroplan(self, ra, dec, location=None, time=None):
        """
        Calculate the parallactic angle for a given RA and Dec using Astroplan.
        @param ra: Right Ascension as a space-separated string (e.g., "23 08 44.55").
        @param dec: Declination as a space-separated string (e.g., "+36 22 12.90").
        @param location: Observatory location (Astropy EarthLocation). Defaults to Palomar Observatory.
        @param time: Observation time (Astropy Time). Defaults to current UTC time.
        @return: Parallactic Angle (Astropy Quantity, angle with unit)
        """
        
        # Check if location is None and set default location
        if location is None:
            print("No location provided, using default Palomar Observatory location.")
            location = EarthLocation(lat=33.3563 * u.deg, lon=-116.8648 * u.deg, height=1706 * u.m)
        else:
            print(f"Location provided: {location}")
        
        # Debugging: Check if location is an instance of EarthLocation
        print(f"Type of location: {type(location)}")
        if not isinstance(location, EarthLocation):
            raise TypeError(f"Expected location to be an instance of EarthLocation, but got {type(location)}")
        
        # Create an Observer instance with the location
        try:
            observer = Observer(location=location, name="Observer", timezone="UTC")
            print(f"Observer created with location: {observer.location}")
        except Exception as e:
            print(f"Error creating observer: {e}")
            raise
        
        # Default time: current UTC time if not provided
        if time is None:
            print("No time provided, using current UTC time.")
            time = Time.now()
        else:
            print(f"Time provided: {time}")
        
        # Debugging: Check if time is an instance of astropy.time.Time
        print(f"Type of time: {type(time)}")
        if not isinstance(time, Time):
            raise TypeError(f"Expected time to be an instance of astropy.time.Time, but got {type(time)}")
        
        # Format RA and Dec properly (e.g., "23 08 44.55" -> "23h 08m 44.55s")
        print(f"Original RA: {ra}")
        print(f"Original Dec: {dec}")
        
        ra = ra.replace(" ", "h", 1).replace(" ", "m", 1) + "s"
        dec = dec.replace(" ", "d", 1).replace(" ", "m", 1) + "s"
        
        print(f"Formatted RA: {ra}")
        print(f"Formatted Dec: {dec}")
        
        # Convert RA and Dec to SkyCoord
        try:
            target_coords = SkyCoord(ra=ra, dec=dec, frame='icrs')
            print(f"SkyCoord for target: {target_coords}")
        except Exception as e:
            print(f"Error creating SkyCoord: {e}")
            raise
        
        # Calculate the parallactic angle
        try:
            parallactic_angle = observer.parallactic_angle(time, target_coords)
            print(f"Parallactic angle: {parallactic_angle}")
        except Exception as e:
            print(f"Error calculating parallactic angle: {e}")
            raise
        
        return f"{parallactic_angle.to(u.deg).value:.2f}"