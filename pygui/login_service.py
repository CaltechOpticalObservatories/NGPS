import mysql.connector
from PyQt5.QtWidgets import QDialog, QLineEdit, QVBoxLayout, QPushButton, QLabel, QFormLayout, QMessageBox
from PyQt5.QtCore import Qt
import re

class LoginService:
    def __init__(self, parent):
        self.parent = parent
        self.connection = None

    def connect_to_db(self):
        """
        Establish connection to the MySQL database using LogicService.
        """
        try:
            self.connection = self.parent.logic_service.connect_to_mysql(
                "config/db_config.ini"
            )
            return self.connection is not None

        except mysql.connector.Error as err:
            print(f"Database connection error: {err}")
            self.connection = None
            return False

        except Exception as e:
            print(f"Unexpected database connection error: {e}")
            self.connection = None
            return False

    def create_account(self, owner_id, password, confirmPassword, email):
        """
        Create a new user account in the owner table.

        owner table schema:
            OWNER_ID varchar(64) primary key
            PASSWORD varchar(64)
            EMAIL varchar(128)
        """
        owner_id = owner_id.strip()
        email = email.strip()

        if not owner_id:
            self.show_error_message("Username is required.")
            return False

        if len(owner_id) > 64:
            self.show_error_message("Username must be 64 characters or fewer.")
            return False

        if not password:
            self.show_error_message("Password is required.")
            return False

        if len(password) > 64:
            self.show_error_message("Password must be 64 characters or fewer.")
            return False

        if password != confirmPassword:
            self.show_error_message("Passwords do not match.")
            return False

        if not email:
            self.show_error_message("Email is required.")
            return False

        if len(email) > 128:
            self.show_error_message("Email must be 128 characters or fewer.")
            return False

        if not self.is_valid_email(email):
            self.show_error_message("Invalid email format.")
            return False

        if not self.connect_to_db():
            self.show_error_message("Could not connect to the database.")
            return False

        cursor = None

        try:
            cursor = self.connection.cursor(dictionary=True)

            cursor.execute(
                "SELECT OWNER_ID FROM owner WHERE OWNER_ID = %s",
                (owner_id,),
            )
            existing_user = cursor.fetchone()

            if existing_user:
                self.show_error_message(
                    f"Username '{owner_id}' already exists. Please choose another."
                )
                return False

            cursor.execute(
                """
                INSERT INTO owner (OWNER_ID, PASSWORD, EMAIL)
                VALUES (%s, %s, %s)
                """,
                (owner_id, password, email),
            )

            self.connection.commit()
            print(f"Account successfully created for user: {owner_id}")
            return True

        except mysql.connector.Error as err:
            print(f"Error creating account: {err}")

            if self.connection:
                self.connection.rollback()

            self.show_error_message(f"Error creating account: {err}")
            return False

        except Exception as e:
            print(f"Unexpected error creating account: {e}")

            if self.connection:
                self.connection.rollback()

            self.show_error_message("Unexpected error creating account.")
            return False

        finally:
            if cursor is not None:
                cursor.close()

            if self.connection is not None:
                self.connection.close()
                self.connection = None

    def login(self, owner_id, password):
        """
        Validate user login against owner table.
        """
        owner_id = owner_id.strip()

        if not owner_id or not password:
            return False

        if not self.connect_to_db():
            return False

        cursor = None

        try:
            cursor = self.connection.cursor(dictionary=True)

            cursor.execute(
                """
                SELECT OWNER_ID
                FROM owner
                WHERE OWNER_ID = %s
                  AND PASSWORD = %s
                """,
                (owner_id, password),
            )

            result = cursor.fetchone()
            return result is not None

        except mysql.connector.Error as err:
            print(f"Login database error: {err}")
            return False

        finally:
            if cursor is not None:
                cursor.close()

            if self.connection is not None:
                self.connection.close()
                self.connection = None

    def is_valid_email(self, email):
        """Check if the email format is valid."""
        email_regex = r'^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$'
        return bool(re.match(email_regex, email))

    def show_error_message(self, message):
        """Show an error message dialog."""
        msg = QMessageBox(self.parent)
        msg.setIcon(QMessageBox.Critical)
        msg.setText(message)
        msg.setWindowTitle("Error")
        msg.exec_()


class LoginDialog(QDialog):
    def __init__(self, parent=None, connection=None):
        super().__init__(parent)
        self.setWindowTitle("Login")

        # Store the actual main window reference.
        # Do not use self.parent because QDialog already has a parent() method.
        self.main_window = parent

        # Database connection information passed as parameter
        self.connection = connection

        self.username_field = QLineEdit(self)
        self.password_field = QLineEdit(self)
        self.password_field.setEchoMode(QLineEdit.Password)

        self.login_button = QPushButton("Login", self)
        self.cancel_button = QPushButton("Cancel", self)
        self.account_button = QPushButton("Create Account", self)
        self.forgot_button = QPushButton("Forgot Password", self)

        # Layout
        layout = QFormLayout()
        layout.addRow("Username:", self.username_field)
        layout.addRow("Password:", self.password_field)
        layout.addRow(self.login_button, self.cancel_button)
        layout.addRow(self.account_button, self.forgot_button)
        self.setLayout(layout)

        # Connect signals
        self.login_button.clicked.connect(self.on_login)
        self.cancel_button.clicked.connect(self.reject)
        self.account_button.clicked.connect(self.on_create_account)

        self.owner = None
        self.all_targets = []
        self.set_data = {}
        self.set_name = []

    def on_login(self):
        """Handles the login action."""
        username = self.username_field.text().strip()
        password = self.password_field.text()

        if self.validate_user_credentials(username, password):
            print(f"Login successful for user: {username}")

            self.owner = username
            self.fetch_and_update_target_list(username)

            self.accept()
        else:
            print(f"Login failed for user: {username}")
            self.show_error_message("Invalid credentials, please try again.")

    def on_cancel(self):
        """Handles the cancel action."""
        print("Login cancelled")
        self.reject()

    def on_create_account(self):
        """
        Close the login dialog and open the create-account dialog
        from the main window.
        """
        self.reject()

        if self.main_window is not None:
            self.main_window.on_create_account()

    def validate_user_credentials(self, username, password):
        """Validate user credentials against the MySQL database."""
        if not username or not password:
            return False

        try:
            print(f"Attempting to validate credentials for username: {username}")

            cursor = self.connection.cursor(dictionary=True)

            cursor.execute(
                """
                SELECT OWNER_ID
                FROM owner
                WHERE OWNER_ID = %s
                  AND PASSWORD = %s
                """,
                (username, password),
            )

            user = cursor.fetchone()
            cursor.close()

            if user:
                print(f"Login validated for user: {user['OWNER_ID']}")
                return True

            print(f"Invalid username or password for user: {username}")
            return False

        except mysql.connector.Error as err:
            print(f"Database error: {err}")
            self.show_error_message("An error occurred while connecting to the database.")
            return False

        except Exception as e:
            print(f"Unexpected error: {e}")
            self.show_error_message("An unexpected error occurred.")
            return False

    def show_error_message(self, message):
        """Display error message in the dialog."""
        QMessageBox.critical(self, "Error", message)

    def fetch_and_update_target_list(self, username):
        """Fetch target data and update the table in the parent window."""
        if self.connection:
            try:
                cursor = self.connection.cursor(dictionary=True)

                cursor.execute(
                    "SELECT SET_ID FROM target_sets WHERE OWNER = %s",
                    (username,),
                )
                set_ids = cursor.fetchall()

                cursor.execute(
                    "SELECT SET_ID, SET_NAME FROM target_sets WHERE OWNER = %s",
                    (username,),
                )
                set_data = cursor.fetchall()

                self.set_data = {
                    set_item["SET_ID"]: set_item["SET_NAME"]
                    for set_item in set_data
                }

                self.set_name = [
                    set_item["SET_NAME"]
                    for set_item in set_data
                ]

                self.all_targets = []

                for set_id in set_ids:
                    cursor.execute(
                        "SELECT * FROM targets WHERE SET_ID = %s",
                        (set_id["SET_ID"],),
                    )
                    targets = cursor.fetchall()
                    self.all_targets.extend(targets)

                cursor.close()

            except mysql.connector.Error as err:
                print(f"Database error: {err}")

            finally:
                self.connection.close()


class CreateAccountDialog(QDialog):
    def __init__(self, parent):
        super().__init__(parent)
        self.setWindowTitle("Create Account")
        self.parent = parent
        
        # Create the form inputs
        self.owner_id = QLineEdit(self)
        self.password = QLineEdit(self)
        self.password.setEchoMode(QLineEdit.Password)
        self.confirmPassword = QLineEdit(self)
        self.confirmPassword.setEchoMode(QLineEdit.Password)
        self.email = QLineEdit(self)
        
        self.create_account_button = QPushButton("Create Account", self)
        self.create_account_button.clicked.connect(self.handle_create_account)

        layout = QFormLayout()
        layout.addRow("Username:", self.owner_id)
        layout.addRow("Password:", self.password)
        layout.addRow("Confirm Password:", self.confirmPassword)
        layout.addRow("Email:", self.email)
        layout.addWidget(self.create_account_button)
        
        self.setLayout(layout)

    def handle_create_account(self):
        """Handle account creation logic."""
        owner_id = self.owner_id.text().strip()
        password = self.password.text()
        confirmPassword = self.confirmPassword.text()
        email = self.email.text().strip()

        login_service = LoginService(self.parent)

        if login_service.create_account(owner_id, password, confirmPassword, email):
            QMessageBox.information(
                self,
                "Account Created",
                f"Account '{owner_id}' was created successfully."
            )
            self.accept()

    def show_error_message(self, message):
        """Show an error message dialog."""
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Critical)
        msg.setText(message)
        msg.setWindowTitle("Error")
        msg.exec_()
