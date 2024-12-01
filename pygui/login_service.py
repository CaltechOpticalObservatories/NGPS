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
            self.connection = mysql.connector.connect(
                host=self.db_config["host"],
                user=self.db_config["user"],
                password=self.db_config["password"],
                database=self.db_config["database"]
            )
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
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Login")

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
        username = self.username_field.text()
        password = self.password_field.text()

        # Validate user credentials (you can expand this logic to interact with your database)
        if self.validate_user_credentials(username, password):
            self.accept()  # Close the dialog on success
        else:
            self.show_error_message("Invalid credentials, please try again.")

    def validate_user_credentials(self, username, password):
        # Here you can validate the credentials against your database.
        # For now, we'll assume a simple validation.
        # Ideally, you'd query the database to check credentials.
        if username == "test_user" and password == "password123":
            return True
        return False

    def show_error_message(self, message):
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
