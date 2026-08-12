import bcrypt
import mysql.connector
from PyQt5.QtWidgets import QDialog, QLineEdit, QVBoxLayout, QPushButton, QLabel, QFormLayout, QMessageBox
from PyQt5.QtCore import Qt
import re


def hash_password(password):
    """Hash a plaintext password for storage."""
    return bcrypt.hashpw(password.encode("utf-8"), bcrypt.gensalt()).decode("utf-8")


def is_legacy_plaintext(stored_password):
    """True if stored_password predates password hashing (not a bcrypt hash)."""
    return bool(stored_password) and not stored_password.startswith("$2")


def verify_password(password, stored_password):
    """Check a password against its stored bcrypt hash, or legacy plaintext."""
    if not stored_password:
        return False

    if is_legacy_plaintext(stored_password):
        return password == stored_password

    return bcrypt.checkpw(password.encode("utf-8"), stored_password.encode("utf-8"))


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
                (owner_id, hash_password(password), email),
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
                SELECT PASSWORD
                FROM owner
                WHERE OWNER_ID = %s
                """,
                (owner_id,),
            )

            result = cursor.fetchone()

            if result is None:
                return False

            return verify_password(password, result["PASSWORD"])

        except mysql.connector.Error as err:
            print(f"Login database error: {err}")
            return False

        finally:
            if cursor is not None:
                cursor.close()

            if self.connection is not None:
                self.connection.close()
                self.connection = None

    def change_password(self, owner_id, current_password, new_password, confirm_new_password):
        """
        Change a user's password after verifying their current password.
        """
        owner_id = owner_id.strip()

        if not owner_id:
            self.show_error_message("Username is required.")
            return False

        if not current_password:
            self.show_error_message("Current password is required.")
            return False

        if not new_password:
            self.show_error_message("New password is required.")
            return False

        if len(new_password) > 64:
            self.show_error_message("Password must be 64 characters or fewer.")
            return False

        if new_password != confirm_new_password:
            self.show_error_message("New passwords do not match.")
            return False

        if new_password == current_password:
            self.show_error_message("New password must be different from the current password.")
            return False

        if not self.connect_to_db():
            self.show_error_message("Could not connect to the database.")
            return False

        cursor = None

        try:
            cursor = self.connection.cursor(dictionary=True)

            cursor.execute(
                "SELECT PASSWORD FROM owner WHERE OWNER_ID = %s",
                (owner_id,),
            )
            user = cursor.fetchone()

            if not user or not verify_password(current_password, user["PASSWORD"]):
                self.show_error_message("Username or current password is incorrect.")
                return False

            cursor.execute(
                "UPDATE owner SET PASSWORD = %s WHERE OWNER_ID = %s",
                (hash_password(new_password), owner_id),
            )
            self.connection.commit()

            print(f"Password successfully changed for user: {owner_id}")
            return True

        except mysql.connector.Error as err:
            print(f"Error changing password: {err}")

            if self.connection:
                self.connection.rollback()

            self.show_error_message(f"Error changing password: {err}")
            return False

        except Exception as e:
            print(f"Unexpected error changing password: {e}")

            if self.connection:
                self.connection.rollback()

            self.show_error_message("Unexpected error changing password.")
            return False

        finally:
            if cursor is not None:
                cursor.close()

            if self.connection is not None:
                self.connection.close()
                self.connection = None

    def reset_password(self, owner_id, new_password, confirm_new_password):
        """
        Reset a user's password given only their username, with no proof of
        identity (no current password, no email verification).

        TODO(security): this is a placeholder "forgot password" flow. Anyone
        who knows a username can currently reset that account's password.
        Replace with a real verification step (e.g. emailed reset token)
        before this is used outside of a trusted/internal deployment.
        """
        owner_id = owner_id.strip()

        if not owner_id:
            self.show_error_message("Username is required.")
            return False

        if not new_password:
            self.show_error_message("New password is required.")
            return False

        if len(new_password) > 64:
            self.show_error_message("Password must be 64 characters or fewer.")
            return False

        if new_password != confirm_new_password:
            self.show_error_message("New passwords do not match.")
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
            user = cursor.fetchone()

            if not user:
                self.show_error_message(f"No account found for username '{owner_id}'.")
                return False

            cursor.execute(
                "UPDATE owner SET PASSWORD = %s WHERE OWNER_ID = %s",
                (hash_password(new_password), owner_id),
            )
            self.connection.commit()

            print(f"Password reset for user: {owner_id}")
            return True

        except mysql.connector.Error as err:
            print(f"Error resetting password: {err}")

            if self.connection:
                self.connection.rollback()

            self.show_error_message(f"Error resetting password: {err}")
            return False

        except Exception as e:
            print(f"Unexpected error resetting password: {e}")

            if self.connection:
                self.connection.rollback()

            self.show_error_message("Unexpected error resetting password.")
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
        self.forgot_button.clicked.connect(self.on_forgot_password)

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

    def on_forgot_password(self):
        """
        Close the login dialog and open the forgot-password dialog
        from the main window.
        """
        self.reject()

        if self.main_window is not None:
            self.main_window.on_forgot_password()

    def validate_user_credentials(self, username, password):
        """Validate user credentials against the MySQL database."""
        if not username or not password:
            return False

        try:
            print(f"Attempting to validate credentials for username: {username}")

            cursor = self.connection.cursor(dictionary=True)

            cursor.execute(
                """
                SELECT OWNER_ID, PASSWORD
                FROM owner
                WHERE OWNER_ID = %s
                """,
                (username, ),
            )

            user = cursor.fetchone()
            cursor.close()

            if user and self._check_password(username, password, user["PASSWORD"]):
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

    def _check_password(self, username, password, stored_password):
        """
        Verify a password against its stored hash. Accounts created before
        password hashing was added still have a plaintext PASSWORD; if one of
        those matches, transparently upgrade it to a bcrypt hash.
        """
        if not verify_password(password, stored_password):
            return False

        if is_legacy_plaintext(stored_password):
            self._upgrade_password_hash(username, password)

        return True

    def _upgrade_password_hash(self, username, password):
        """Rehash a legacy plaintext password and store it as a bcrypt hash."""
        try:
            cursor = self.connection.cursor()
            cursor.execute(
                "UPDATE owner SET PASSWORD = %s WHERE OWNER_ID = %s",
                (hash_password(password), username),
            )
            self.connection.commit()
            cursor.close()

            print(f"Upgraded password hash for user: {username}")

        except mysql.connector.Error as err:
            print(f"Failed to upgrade password hash for user {username}: {err}")

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


class ChangePasswordDialog(QDialog):
    def __init__(self, parent):
        super().__init__(parent)
        self.setWindowTitle("Change Password")
        self.parent = parent

        # Create the form inputs
        self.owner_id = QLineEdit(self)

        current_owner = getattr(parent, "current_owner", None)
        if current_owner:
            self.owner_id.setText(current_owner)

        self.current_password = QLineEdit(self)
        self.current_password.setEchoMode(QLineEdit.Password)
        self.new_password = QLineEdit(self)
        self.new_password.setEchoMode(QLineEdit.Password)
        self.confirm_new_password = QLineEdit(self)
        self.confirm_new_password.setEchoMode(QLineEdit.Password)

        self.change_password_button = QPushButton("Change Password", self)
        self.change_password_button.clicked.connect(self.handle_change_password)

        layout = QFormLayout()
        layout.addRow("Username:", self.owner_id)
        layout.addRow("Current Password:", self.current_password)
        layout.addRow("New Password:", self.new_password)
        layout.addRow("Confirm New Password:", self.confirm_new_password)
        layout.addWidget(self.change_password_button)

        self.setLayout(layout)

    def handle_change_password(self):
        """Handle change-password logic."""
        owner_id = self.owner_id.text().strip()
        current_password = self.current_password.text()
        new_password = self.new_password.text()
        confirm_new_password = self.confirm_new_password.text()

        login_service = LoginService(self.parent)

        if login_service.change_password(
            owner_id, current_password, new_password, confirm_new_password
        ):
            QMessageBox.information(
                self,
                "Password Changed",
                f"Password for '{owner_id}' was changed successfully."
            )
            self.accept()

    def show_error_message(self, message):
        """Show an error message dialog."""
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Critical)
        msg.setText(message)
        msg.setWindowTitle("Error")
        msg.exec_()


class ForgotPasswordDialog(QDialog):
    """
    NOTE: this resets a password from the username alone, with no proof of
    identity. It's a stand-in for now, not a real "forgot password" flow --
    see LoginService.reset_password for the security caveat.
    """

    def __init__(self, parent):
        super().__init__(parent)
        self.setWindowTitle("Forgot Password")
        self.parent = parent

        # Create the form inputs
        self.owner_id = QLineEdit(self)
        self.new_password = QLineEdit(self)
        self.new_password.setEchoMode(QLineEdit.Password)
        self.confirm_new_password = QLineEdit(self)
        self.confirm_new_password.setEchoMode(QLineEdit.Password)

        self.reset_password_button = QPushButton("Reset Password", self)
        self.reset_password_button.clicked.connect(self.handle_reset_password)

        layout = QFormLayout()
        layout.addRow("Username:", self.owner_id)
        layout.addRow("New Password:", self.new_password)
        layout.addRow("Confirm New Password:", self.confirm_new_password)
        layout.addWidget(self.reset_password_button)

        self.setLayout(layout)

    def handle_reset_password(self):
        """Handle forgot-password logic."""
        owner_id = self.owner_id.text().strip()
        new_password = self.new_password.text()
        confirm_new_password = self.confirm_new_password.text()

        login_service = LoginService(self.parent)

        if login_service.reset_password(owner_id, new_password, confirm_new_password):
            QMessageBox.information(
                self,
                "Password Reset",
                f"Password for '{owner_id}' was reset successfully. "
                "You can now log in with your new password."
            )
            self.accept()

    def show_error_message(self, message):
        """Show an error message dialog."""
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Critical)
        msg.setText(message)
        msg.setWindowTitle("Error")
        msg.exec_()
