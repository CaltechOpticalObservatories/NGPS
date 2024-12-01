import mysql.connector
from PyQt5.QtWidgets import QDialog, QLineEdit, QVBoxLayout, QPushButton, QLabel, QFormLayout, QMessageBox
from PyQt5.QtCore import Qt
import re

class LoginService:
    def __init__(self, parent):
        self.parent = parent
        self.db_config = parent.db_config  # This should contain the MySQL config
        self.connection = None

    def connect_to_db(self):
        """Establish connection to the MySQL database."""
        try:
            self.parent.logic_service.connect_to_mysql("config/db_config.ini")
        except mysql.connector.Error as err:
            print(f"Error: {err}")
            return False
        return True

    def login(self, owner_id, password):
        """Validate user login."""
        if not self.connect_to_db():
            return False

        cursor = self.connection.cursor(dictionary=True)
        cursor.execute("SELECT * FROM owner WHERE owner_id = %s AND password = %s", (owner_id, password))
        result = cursor.fetchone()
        
        cursor.close()
        self.connection.close()

        if result:
            return True
        else:
            return False

    def create_account(self, owner_id, password, confirmPassword, email):
        """Create a new user account in the 'owner' table."""
        if not self.connect_to_db():
            return False

        # Ensure the passwords match
        if password != confirmPassword:
            self.show_error_message("Passwords do not match.")
            return False

        # Check if the email is valid
        if not self.is_valid_email(email):
            self.show_error_message("Invalid email format.")
            return False
        
        cursor = self.connection.cursor()
        
        # Insert the new user data
        try:
            cursor.execute("INSERT INTO owner (owner_id, password, email) VALUES (%s, %s, %s)",
                           (owner_id, password, email))
            self.connection.commit()
        except mysql.connector.Error as err:
            print(f"Error: {err}")
            self.show_error_message(f"Error creating account: {err}")
            cursor.close()
            self.connection.close()
            return False
        
        cursor.close()
        self.connection.close()
        return True

    def is_valid_email(self, email):
        """Check if the email format is valid."""
        email_regex = r'^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$'
        return bool(re.match(email_regex, email))

    def show_error_message(self, message):
        """Show an error message dialog."""
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Critical)
        msg.setText(message)
        msg.setWindowTitle("Error")
        msg.exec_()


class LoginDialog(QDialog):
    def __init__(self, parent=None, db_config=None):
        super().__init__(parent)
        self.setWindowTitle("Login")

        # Database connection information passed as parameter
        self.db_config = db_config  # Assumes db_config is passed from NgpsGUI or other parent
        
        self.username_field = QLineEdit(self)
        self.password_field = QLineEdit(self)
        self.password_field.setEchoMode(QLineEdit.Password)

        self.login_button = QPushButton("Login", self)
        self.cancel_button = QPushButton("Cancel", self)

        # Layout
        layout = QFormLayout()
        layout.addRow("Username:", self.username_field)
        layout.addRow("Password:", self.password_field)
        layout.addRow(self.login_button, self.cancel_button)
        self.setLayout(layout)

        # Connect signals
        self.login_button.clicked.connect(self.on_login)
        self.cancel_button.clicked.connect(self.reject)

    def on_login(self):
        """Handles the login action."""
        username = self.username_field.text()
        password = self.password_field.text()

        # Validate user credentials by checking against the database
        if self.validate_user_credentials(username, password):
            print(f"Login successful for user: {username}")
            self.accept()  # Close the dialog on success
        else:
            print(f"Login failed for user: {username}")
            self.show_error_message("Invalid credentials, please try again.")

    def validate_user_credentials(self, username, password):
        """Validate user credentials against the MySQL database."""
        try:
            # Print to show method entry
            print(f"Attempting to validate credentials for username: {username}")

            # Connect to MySQL database
            print(f"Connecting to MySQL database: {self.db_config['DBMS']} at {self.db_config['SYSTEM']}")
            connection = mysql.connector.connect(
                host=self.db_config["SYSTEM"],
                user=self.db_config["USERNAME"],
                password=self.db_config["PASSWORD"],
                database=self.db_config["DBMS"]
            )
            
            # Print confirmation after connection
            print(f"Successfully connected to the MySQL database.")

            cursor = connection.cursor(dictionary=True)
            cursor.execute("SELECT * FROM owner WHERE owner_id = %s", (username,))
            
            # Print the query being executed
            print(f"Executed query: SELECT * FROM owner WHERE owner_id = '{username}'")
            
            user = cursor.fetchone()

            if user:
                print(f"User found: {user['owner_id']}")
                # Check if the password matches
                if user["password"] == password:
                    print(f"Password matches for user: {username}")
                    return True
                else:
                    print(f"Password does not match for user: {username}")
                    return False
            else:
                print(f"No user found with username: {username}")
                return False

        except mysql.connector.Error as err:
            # Log the error if there is a database issue
            print(f"Database error: {err}")
            self.show_error_message("An error occurred while connecting to the database.")
            return False
        except Exception as e:
            # Catch any other unexpected errors
            print(f"Unexpected error: {e}")
            self.show_error_message("An unexpected error occurred.")
            return False

    def show_error_message(self, message):
        """Display error message in the dialog."""
        error_label = QLabel(message, self)
        self.layout().addWidget(error_label)


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
        owner_id = self.owner_id.text()
        password = self.password.text()
        confirmPassword = self.confirmPassword.text()
        email = self.email.text()

        login_service = LoginService(self.parent)

        if login_service.create_account(owner_id, password, confirmPassword, email):
            self.accept()  # Close the dialog on successful account creation
        else:
            self.show_error_message("Account creation failed")

    def show_error_message(self, message):
        """Show an error message dialog."""
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Critical)
        msg.setText(message)
        msg.setWindowTitle("Error")
        msg.exec_()
