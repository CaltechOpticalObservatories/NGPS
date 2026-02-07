import mysql.connector
import configparser
from PyQt5.QtWidgets import QTableWidgetItem
from PyQt5.QtCore import Qt, pyqtSignal, QSignalBlocker
from PyQt5.QtGui import QColor, QFont
import os
import csv
import pytz
from astropy.coordinates import SkyCoord, EarthLocation
from astropy.time import Time
from astroplan import Observer
import astropy.units as u
import datetime
from contextlib import contextmanager  

class LogicService:
    def __init__(self, parent):
        self.parent = parent  # reference to the parent window or main UI
        self.connection = None
        self.all_targets = []
        self.target_list_set = {}
        self.target_list_display = None

    def _connect_unique(self, signal, slot):
        """Disconnect slot if already connected, then connect once."""
        try:
            signal.disconnect(slot)
        except TypeError:
            pass
        signal.connect(slot)

    def _get_target_combo(self):
        """
        Return the QComboBox used for target list selection, whether it lives on
        the main window or inside layout_service. Returns None if not found.
        """
        return getattr(self.parent, "current_target_list_name", None)

    @contextmanager
    def _maybe_block(self, widget):
        """Context manager to block signals if widget is not None."""
        blocker = None
        if widget is not None:
            blocker = QSignalBlocker(widget)
        try:
            yield
        finally:
            if blocker is not None:
                del blocker


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
            connection = self.connect_to_mysql("config/db_config.ini")  # Assuming you have a method to connect to the DB

            if connection is None:
                print("Failed to connect to MySQL. Cannot refresh table.")
                return

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
                'OTMres', 'OTMseeing', 'OTMslitangle', 'NOTE', 'COMMENT', 'OWNER', 'NOTBEFORE', 'POINTMODE', 'PRIORITY'
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
                    if value is None or value == '':
                        # Numeric columns (defaults to 0)
                        if column in ['OBS_ORDER', 'TARGET_NUMBER', 'SEQUENCE_NUMBER']:
                            value = 0
                        elif column in ['BINSPECT']:
                            value = 1
                        elif column in ['BINSPAT']:
                            value = 2
                        elif column in ['STATE']:
                            value = "pending"  # Empty string as default for text fields
                        elif column in ['STATE', 'RA', 'DECL', 'EXPTIME', 'SLITWIDTH']:
                            value = ""  # Empty string as default for text fields
                        # Timestamp columns (defaults to NULL)
                        elif column in ['NOTBEFORE', 'OTMslewgo', 'OTMexp_start', 'OTMexp_end']:
                            value = None  # Default to NULL for timestamps
                        else:
                            value = None  # Default to NULL for other columns without a defined default

                    # Special case: if `OFFSET_RA` or `OFFSET_DEC` are empty, set them to 0.0
                    if column in ['OFFSET_RA', 'OFFSET_DEC'] and (value == '' or value is None):
                        value = 0.0  # Default to 0.0 if empty or None

                    # Special case: if "PRIORITY" is empty, set it to "1"
                    if column == "PRIORITY" and (value == '' or value is None):
                        value = "1"  # Default to "1" if empty or None

                    # Special case: OTMslitwidth is NOT NULL in schema, default to 0.0
                    if column == "OTMslitwidth" and (value == '' or value is None):
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

            print(f"Successfully uploaded {len(data)} targets to the new set {target_set_name}.")
            # Emit the signal after the upload is complete
            self.fetch_and_update_target_list()

        except mysql.connector.Error as err:
            print(f"Error inserting data into MySQL: {err}")
            self.parent.show_popup("Error uploading target list! Please try again.")

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

                # Handle required NOT NULL columns with defaults
                if 'OTMslitwidth' in row and (row['OTMslitwidth'] == '' or row['OTMslitwidth'] is None):
                    row['OTMslitwidth'] = 0.0
                if 'PRIORITY' in row and (row['PRIORITY'] == '' or row['PRIORITY'] is None):
                    row['PRIORITY'] = "1"
                if 'OFFSET_RA' in row and (row['OFFSET_RA'] == '' or row['OFFSET_RA'] is None):
                    row['OFFSET_RA'] = 0.0
                if 'OFFSET_DEC' in row and (row['OFFSET_DEC'] == '' or row['OFFSET_DEC'] is None):
                    row['OFFSET_DEC'] = 0.0

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
        Prevents duplicate signal connections and cascading refreshes.
        """
        connection = self.connect_to_mysql(config_file)
        if connection is None:
            print("Failed to connect to MySQL. Cannot load target data.")
            return

        db_config = self.read_config(config_file)
        target_table = db_config.get("TARGET_TABLE")
        rows = self.load_data_from_mysql(connection, target_table)
        if not rows:
            print(f"No data found in the {target_table} table.")
            return

        # Update table (idempotent + signal-safe)
        self.update_target_list_table(rows)

        # Build / refresh the combo quietly, then wire the filter once
        combo = self._get_target_combo()
        with self._maybe_block(combo):
            if combo is not None:
                target_list_names = sorted({str(row.get('SET_ID')) for row in rows if 'SET_ID' in row})
                combo.clear()
                combo.addItem("All")
                combo.addItems(target_list_names)

        if combo is not None:
            self._connect_unique(combo.currentIndexChanged, self.filter_target_list)


    def load_mysql_and_fetch_target_sets(self, config_file):
        """
        Load ALL target sets for current user (newest first), but auto-select the most recent.
        Returns a list of SET_NAMEs for the combo; stores {SET_ID: SET_NAME} mapping on parent.
        """
        username = getattr(self.parent, "current_owner", None)
        if not username:
            print("No owner information found. Cannot load target sets.")
            return []

        conn = self.connect_to_mysql(config_file)
        if conn is None:
            print("Failed to connect to MySQL.")
            return []

        try:
            cur = conn.cursor(dictionary=True)
            cur.execute("""
                SELECT SET_ID, SET_NAME,
                    COALESCE(SET_CREATION_TIMESTAMP,'1970-01-01 00:00:00') AS ts
                FROM target_sets
                WHERE OWNER = %s
                ORDER BY ts DESC, SET_ID DESC
            """, (username,))
            sets = cur.fetchall()
            cur.close()

            if not sets:
                print(f"No target sets found for user '{username}'.")
                self.set_data = {}
                self.set_name = []
                setattr(self.parent, "user_set_data", {})
                return []

            # Map + list of names
            self.set_data = {row["SET_ID"]: row["SET_NAME"] for row in sets}
            self.set_name = [row["SET_NAME"] for row in sets]
            self.parent.user_set_data = self.set_data

            # Auto-select the newest by name (the first row)
            self.parent.current_target_list_name = sets[0]["SET_NAME"]

            return self.set_name

        except mysql.connector.Error as err:
            print(f"Database error: {err}")
            return []
        finally:
            conn.close()


    def load_calibration_target_sets(self, config_file):
        """
        Loads calibration target sets (SET_NAME starting with 'CAL_') for the current user.
        Returns a list of filtered SET_NAMEs for UI population.
        """
        username = getattr(self.parent, "current_owner", None)
        if not username:
            print("No owner information found. Cannot load calibration target sets.")
            return []

        connection = self.connect_to_mysql(config_file)
        if connection is None:
            print("Failed to connect to MySQL.")
            return []

        try:
            cursor = connection.cursor(dictionary=True)
            cursor.execute(
                "SELECT SET_ID, SET_NAME FROM target_sets WHERE OWNER = %s AND SET_NAME LIKE 'CAL\\_%'",
                (username,)
            )
            set_data = cursor.fetchall()

            if not set_data:
                print(f"No calibration sets found for user '{username}'.")
                return []

            self.set_data = {row["SET_ID"]: row["SET_NAME"] for row in set_data}
            self.set_name = [row["SET_NAME"] for row in set_data]

            self.all_targets = []
            for row in set_data:
                cursor.execute("SELECT * FROM targets WHERE SET_ID = %s", (row["SET_ID"],))
                targets = cursor.fetchall()
                self.all_targets.extend(targets)

            print(f"Fetched {len(self.set_name)} calibration target sets.")
            return self.set_name

        except mysql.connector.Error as err:
            print(f"Database error: {err}")
            return []
        finally:
            connection.close()

        
    def fetch_set_id(self, target_list=None):
        """
        Resolve the current target list to a SET_ID using parent.current_target_list_name
        (or an explicit target_list arg). Handles numeric IDs or names.
        """
        # Use explicit arg if provided; otherwise read the string you store on the parent
        name_or_id = target_list
        if name_or_id is None:
            name_or_id = getattr(self.parent, "current_target_list_name", None)

        if name_or_id is None:
            return None

        s = str(name_or_id).strip()

        # Ignore non-real selections/sentinels
        if s in ("All", "Create a new target list", "No Target Lists Available", ""):
            return None

        # If it's already an ID (string of digits or int), return it
        if isinstance(name_or_id, int) or s.isdigit():
            try:
                return int(name_or_id)
            except Exception:
                return int(s)

        # Try in-memory mapping first: {SET_ID: SET_NAME}
        mapping = getattr(self.parent, "user_set_data", {}) or {}
        for sid, name in mapping.items():
            if str(name).strip().lower() == s.lower():
                try:
                    return int(sid)
                except Exception:
                    return sid  # sid may already be int

        # Last resort: DB lookup by (OWNER, SET_NAME)
        owner = getattr(self.parent, "current_owner", None)
        if not owner:
            return None

        conn = self.connect_to_mysql("config/db_config.ini")
        if not conn:
            return None

        try:
            cur = conn.cursor()
            cur.execute(
                "SELECT SET_ID FROM target_sets WHERE OWNER = %s AND SET_NAME = %s LIMIT 1",
                (owner, s)
            )
            row = cur.fetchone()
            cur.close()
            if row:
                return int(row[0])
        except Exception as e:
            print("fetch_set_id DB lookup failed:", e)

        return None


    def fetch_and_update_target_list(self):
        """After creating/uploading a set: refresh ALL sets, select newest, and show its rows."""
        username = getattr(self.parent, "current_owner", None)
        if not username:
            print("No owner information found. Cannot fetch target list.")
            return

        conn = self.connect_to_mysql("config/db_config.ini")
        if not conn:
            print("Failed to connect to MySQL. Cannot fetch target list.")
            return

        try:
            cur = conn.cursor(dictionary=True)
            cur.execute("""
                SELECT SET_ID, SET_NAME,
                    COALESCE(SET_CREATION_TIMESTAMP,'1970-01-01 00:00:00') AS ts
                FROM target_sets
                WHERE OWNER = %s
                ORDER BY ts DESC, SET_ID DESC
            """, (username,))
            sets = cur.fetchall()

            if not sets:
                self.set_data = {}
                self.set_name = []
                self.parent.user_set_data = {}
                if hasattr(self.parent, "layout_service"):
                    self.parent.layout_service.load_target_lists([])
                self.update_target_list_table([])
                return

            # Update mapping + names (ALL sets)
            self.set_data = {row["SET_ID"]: row["SET_NAME"] for row in sets}
            self.set_name = [row["SET_NAME"] for row in sets]
            self.parent.user_set_data = self.set_data

            # Select newest
            latest = sets[0]
            self.parent.current_target_list_name = latest["SET_NAME"]

            # Update combo with ALL sets
            if hasattr(self.parent, "layout_service"):
                self.parent.layout_service.load_target_lists(self.set_name)

            # Show rows for newest set
            cur2 = conn.cursor(dictionary=True)
            cur2.execute("SELECT * FROM targets WHERE SET_ID = %s", (latest["SET_ID"],))
            rows = cur2.fetchall()
            cur2.close()
            self.update_target_list_table(rows)

            print(f"Showing latest set '{latest['SET_NAME']}' with {len(rows)} rows; {len(self.set_name)} sets in combo.")
        except mysql.connector.Error as err:
            print(f"Database error: {err}")
        finally:
            conn.close()


    def insert_target_to_db(self, target_name, ra, decl,
                            offset_ra=None, offset_dec=None,
                            exptime=None, slitwidth=None, magnitude=None):
        """
        Insert or update a target in ngps.targets for the current SET_ID.
        Uses an UPSERT to avoid duplicates if called multiple times.
        Requires a UNIQUE key (e.g., (SET_ID, Name, RA, Decl)).
        """
        if not target_name or not ra or not decl:
            print("Error: Name, RA, and Decl are required fields.")
            return

        set_id = self.fetch_set_id()
        if set_id is None:
            print("Error: Unable to fetch set_id. No matching target list found.")
            return

        # Reuse an existing connection if possible
        conn = getattr(self, "connection", None)
        if conn is None or not conn.is_connected():
            conn = self.connect_to_mysql("config/db_config.ini")
            if conn is None:
                print("Failed to connect to MySQL. Cannot insert target data.")
                return
            close_after = True
        else:
            close_after = False

        try:
            cursor = conn.cursor()
            query = """
            INSERT INTO ngps.targets
                (SET_ID, Name, RA, Decl, Offset_RA, Offset_Dec, EXPTime, Slitwidth, Magnitude)
            VALUES
                (%s, %s, %s, %s, %s, %s, %s, %s, %s)
            ON DUPLICATE KEY UPDATE
                Offset_RA = VALUES(Offset_RA),
                Offset_Dec = VALUES(Offset_Dec),
                EXPTime    = VALUES(EXPTime),
                Slitwidth  = VALUES(Slitwidth),
                Magnitude  = VALUES(Magnitude)
            """
            data = (
                set_id, target_name, ra, decl,
                offset_ra if offset_ra is not None else None,
                offset_dec if offset_dec is not None else None,
                exptime if exptime is not None else None,
                slitwidth if slitwidth is not None else None,
                magnitude if magnitude is not None else None
            )
            cursor.execute(query, data)
            conn.commit()
            print(f"Upserted target '{target_name}' in SET_ID {set_id}.")
        except mysql.connector.IntegrityError as err:
            # Fires if the UNIQUE key is different from the values you expect
            print(f"Integrity error on upsert: {err}")
        except mysql.connector.Error as err:
            print(f"Error executing insert/upsert query: {err}")
        finally:
            try:
                cursor.close()
            except Exception:
                pass
            if close_after:
                try:
                    conn.close()
                except Exception:
                    pass


    def filter_target_list(self):
        """
        On set selection, fetch targets for that set (or all sets) for the current user.
        Works whether the combo shows SET_IDs or SET_NAMEs.
        """
        combo = self.parent.current_target_list_name
        selected = combo if combo is not None else "All"
        owner = getattr(self.parent, "current_owner", None)

        if not owner:
            print("No current_owner set; cannot fetch targets.")
            return

        conn = self.connect_to_mysql("config/db_config.ini")
        if conn is None:
            print("Failed to connect to MySQL. Cannot fetch targets for selected set.")
            return

        try:
            if selected == "All":
                print("WHAT!?")
                sql = """
                    SELECT t.*
                    FROM targets t
                    INNER JOIN target_sets s ON s.SET_ID = t.SET_ID
                    WHERE s.OWNER = %s
                    ORDER BY t.SET_ID, t.NAME
                """
                params = (owner,)
                cur = conn.cursor(dictionary=True)
                print("Executing filter query:", sql, params)
                cur.execute(sql, params)
                rows = cur.fetchall()
                cur.close()
                self.update_target_list_table(rows)
                return

            set_id = None

            # 1) prefer itemData if you stored it
            try:
                idx = combo.currentIndex()
                data = combo.itemData(idx)
                if isinstance(data, (int, str)) and str(data).isdigit():
                    set_id = int(data)
            except Exception:
                pass

            # 2) if the visible text is an ID
            if set_id is None and selected.isdigit():
                set_id = int(selected)

            # 3) try the in-memory mapping {SET_ID: SET_NAME}
            if set_id is None:
                mapping = getattr(self.parent, "user_set_data", {}) or {}
                for k, v in mapping.items():
                    if str(v).strip() == selected:
                        set_id = k
                        break

            # 4) last-resort: DB lookup by name for this owner
            if set_id is None:
                cur = conn.cursor()
                cur.execute(
                    "SELECT SET_ID FROM target_sets WHERE OWNER = %s AND SET_NAME = %s",
                    (owner, selected)
                )
                r = cur.fetchone()
                cur.close()
                if r:
                    set_id = r[0]

            if set_id is None:
                print(f"Could not resolve selection '{selected}' to a SET_ID; leaving table unchanged.")
                return

            sql = """
                SELECT t.*
                FROM targets t
                INNER JOIN target_sets s ON s.SET_ID = t.SET_ID
                WHERE t.SET_ID = %s AND s.OWNER = %s
                ORDER BY t.NAME
            """
            params = (set_id, owner)
            cur = conn.cursor(dictionary=True)
            print("Executing filter query:", sql, params)
            cur.execute(sql, params)
            rows = cur.fetchall()
            cur.close()

            self.update_target_list_table(rows)

        except mysql.connector.Error as err:
            print(f"Database error during filter: {err}")

    def update_target_list_table(self, data):
        """
        Idempotently repopulates the UI table with 'data'.
        Clears first, blocks internal signals during rebuild, and restores sorting after.
        """
        # Access widgets safely
        ls = getattr(self.parent, "layout_service", None)
        if ls is None or not hasattr(ls, "target_list_display"):
            print("layout_service or target_list_display not available yet.")
            return

        table = ls.target_list_display
        self.parent.all_targets = data  # canonical cache for filtering

        columns_to_hide = {
            "SET_ID", "STATE", "OBS_ORDER", "TARGET_NUMBER", "SEQUENCE_NUMBER",
            "SLITOFFSET", "OBSMODE", "AIRMASS_MAX", "WRANGE_LOW", "WRANGE_HIGH",
            "SRCMODEL", "OTMexpt", "OTMslitwidth", "OTMcass", "OTMairmass_start",
            "OTMairmass_end", "OTMsky", "OTMdead", "OTMslewgo", "OTMexp_start",
            "OTMexp_end", "OTMpa", "OTMwait", "OTMflag", "OTMlast", "OTMslew",
            "OTMmoon", "OTMSNR", "OTMres", "OTMseeing", "OTMslitangle",
            "NOTE", "COMMENT", "OWNER", "NOTBEFORE", "POINTMODE"
        }

        rows = data if isinstance(data, list) else [data]
        filtered_rows = []
        for r in rows:
            if isinstance(r, dict):
                filtered_rows.append({k: v for k, v in r.items() if k not in columns_to_hide})

        table_blocker = QSignalBlocker(table)
        try:
            table.setSortingEnabled(False)
            table.clear()             # headers + contents
            table.setRowCount(0)
            table.setColumnCount(0)

            if not filtered_rows:
                return

            headers = list(filtered_rows[0].keys())
            table.setColumnCount(len(headers))
            table.setHorizontalHeaderLabels(headers)

            header_view = table.horizontalHeader()
            header_view.setFont(QFont("Arial", 10, QFont.Normal))

            for row in filtered_rows:
                row_idx = table.rowCount()
                table.insertRow(row_idx)
                for col_idx, key in enumerate(headers):
                    table.setItem(row_idx, col_idx, QTableWidgetItem(str(row.get(key, ""))))

            table.sortItems(0, Qt.AscendingOrder)
        finally:
            table.setSortingEnabled(True)
            del table_blocker


        ls.load_target_button.setVisible(False)
        table.setVisible(True)
        ls.set_column_widths()
        ls.add_row_button.setEnabled(True)

        try:
            self.apply_active_highlight()
        except Exception as _e:
            pass


    def update_target_table_with_list(self, target_list=None):
        """Rebuild table for the selected target list using the same safe path."""
        ls = getattr(self.parent, "layout_service", None)
        if ls is None:
            print("layout_service not available.")
            return

        # Find set_id for target_list
        set_id = None
        for key, val in getattr(self.parent, "user_set_data", {}).items():
            if val == target_list:
                set_id = key
                break
        if set_id is None:
            print("set_id not found for the given target_list")
            return

        src = getattr(self.parent, "all_targets", [])
        filtered = [row for row in src if row.get('SET_ID') == set_id]
        self.update_target_list_table(filtered)

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
            query = f"UPDATE ngps.targets SET {field_name} = %s WHERE observation_id = %s"
            print(query)

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
            cursor.execute("SELECT observation_id, name, exptime, slitwidth FROM ngps.targets")  # Adjust query as needed
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
        Calculate the parallactic angle for a given RA/Dec using Astroplan.

        ra, dec: strings like "01 15 56.19", "+36 00 06.53" (sexagesimal)
                Also tolerates colon-separated; no manual unit suffixes needed.
        location: astropy.coordinates.EarthLocation (defaults to Palomar)
        time: astropy.time.Time (defaults to current UTC)

        Returns: string degrees with 2 decimals (e.g., "123.45")
        """

        # Location (respect provided, else Palomar)
        if location is None:
            print("No location provided, using default Palomar Observatory location.")
            location = EarthLocation(lat=33.3563 * u.deg, lon=-116.8648 * u.deg, height=1706 * u.m)

        print(f"Type of location: {type(location)}")
        if not isinstance(location, EarthLocation):
            raise TypeError(f"Expected EarthLocation, got {type(location)}")

        # Observer
        try:
            observer = Observer(location=location, name="Observer", timezone="UTC")
            print(f"Observer created with location: {observer.location}")
        except Exception as e:
            print(f"Error creating observer: {e}")
            raise

        # Time (respect provided)
        if time is None:
            print("No time provided, using current UTC time.")
            time = Time.now()
        print(f"Type of time: {type(time)}")
        if not isinstance(time, Time):
            raise TypeError(f"Expected astropy.time.Time, got {type(time)}")

        # RA/Dec parsing (no manual string surgery)
        print(f"Original RA: {ra}")
        print(f"Original Dec: {dec}")
        try:
            # Primary: treat RA as hourangle, Dec as degrees (handles 'HH MM SS', 'HH:MM:SS', etc.)
            target_coords = SkyCoord(ra, dec, unit=(u.hourangle, u.deg), frame='icrs')
        except Exception as e1:
            # Fallback: if RA was given in decimal degrees instead of hours
            try:
                target_coords = SkyCoord(ra, dec, unit=(u.deg, u.deg), frame='icrs')
            except Exception as e2:
                print(f"Error creating SkyCoord (hourangle/deg): {e1}\nAlso failed deg/deg: {e2}")
                raise
        print(f"SkyCoord for target: {target_coords.to_string('hmsdms')}")

        # Parallactic angle
        try:
            pa = observer.parallactic_angle(time, target_coords)  # Angle
            pa_deg = pa.to(u.deg).value
            print(f"Parallactic angle: {pa_deg:.2f} deg")
        except Exception as e:
            print(f"Error calculating parallactic angle: {e}")
            raise

        # Return as string with two decimals to match your existing usage
        return f"{pa_deg:.2f}"
    
    def delete_target_list_by_name(self, target_list_name):
        """
        Deletes all rows from the database that belong to the target list with the given name.
        """
        self.connection = self.connect_to_mysql("config/db_config.ini")
        cursor = self.connection.cursor()

        query = "DELETE FROM target_sets WHERE SET_NAME = %s"
        cursor.execute(query, (target_list_name,))
        self.connection.commit()

        deleted_count = cursor.rowcount

        cursor.close()
        self.connection.close()

        return deleted_count

    def create_empty_target_set(self, set_name: str):
        """
        Create a new empty target set for the current user, then refresh UI to show it.
        """
        set_name = (set_name or "").strip()
        if not set_name:
            print("Empty set name; aborting.")
            return

        owner = getattr(self.parent, "current_owner", None)
        if not owner:
            print("No owner; cannot create target set.")
            return

        conn = self.connect_to_mysql("config/db_config.ini")
        if conn is None:
            print("DB connect failed; cannot create target set.")
            return

        try:
            cur = conn.cursor()
            cur.execute(
                "INSERT INTO target_sets (SET_NAME, OWNER, SET_CREATION_TIMESTAMP) VALUES (%s, %s, NOW())",
                (set_name, owner),
            )
            conn.commit()
            cur.close()
            print(f"Created empty target set '{set_name}' for owner '{owner}'.")
            # Show only the most recent set (this will be the one we just created)
            self.fetch_and_update_target_list()
            # Keep the name around for fetch_set_id callers that read it
            setattr(self.parent, "current_target_list_name", set_name)
        except Exception as e:
            print("create_empty_target_set failed:", e)

    def set_active_target(self, observation_id):
        """Remember the active obs_id and update row highlight."""
        prev = getattr(self.parent, "active_observation_id", None)
        if prev != observation_id:
            setattr(self.parent, "prev_active_observation_id", prev)
        setattr(self.parent, "active_observation_id", observation_id)

        # Update UI highlight now
        self.clear_previous_active_highlight()
        self.apply_active_highlight()

    def _obs_id_column_index(self, table):
        """Find the Observation ID column (case-insensitive)."""
        cols = table.columnCount()
        for i in range(cols):
            item = table.horizontalHeaderItem(i)
            if not item:
                continue
            name = item.text().strip().lower()
            if name in ("observation_id", "obs_id", "observationid"):
                return i
        return None

    def _find_row_by_obs_id(self, table, obs_id):
        """Return the row index with matching observation_id (string compare)."""
        col = self._obs_id_column_index(table)
        if col is None:
            return None
        target = str(obs_id)
        for r in range(table.rowCount()):
            cell = table.item(r, col)
            if cell and cell.text().strip() == target:
                return r
        return None

    def clear_previous_active_highlight(self):
        """Remove yellow highlight from the previously active row, if any."""
        table = getattr(self.parent.layout_service, "target_list_display", None)
        if table is None:
            return
        prev_id = getattr(self.parent, "prev_active_observation_id", None)
        if prev_id is None:
            return
        row = self._find_row_by_obs_id(table, prev_id)
        if row is None:
            return
        for c in range(table.columnCount()):
            item = table.item(row, c)
            if item:
                # reset to default background
                item.setBackground(Qt.white)
                item.setForeground(Qt.black)

    def apply_active_highlight(self):
        """Paint the active row yellow (soft) if visible."""
        table = getattr(self.parent.layout_service, "target_list_display", None)
        if table is None:
            return
        active_id = getattr(self.parent, "active_observation_id", None)
        if active_id is None:
            return
        row = self._find_row_by_obs_id(table, active_id)
        if row is None:
            return
        yellow = QColor(255, 204, 64)
        for c in range(table.columnCount()):
            item = table.item(row, c)
            if item:
                item.setBackground(yellow)
                item.setForeground(Qt.black)