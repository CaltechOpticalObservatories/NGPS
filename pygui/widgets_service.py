from PyQt5.QtWidgets import QProgressBar, QLineEdit, QTextEdit, QTableWidget, QComboBox, QDateTimeEdit, QPushButton

class WidgetsService:
    @staticmethod
    def create_progress_bar():
        progress_bar = QProgressBar()
        progress_bar.setRange(0, 100)
        return progress_bar

    @staticmethod
    def create_line_edit(default_text=""):
        line_edit = QLineEdit(default_text)
        return line_edit

    @staticmethod
    def create_text_edit():
        text_edit = QTextEdit()
        return text_edit

    @staticmethod
    def create_table_widget():
        table_widget = QTableWidget()
        table_widget.setRowCount(10)
        table_widget.setColumnCount(3)
        return table_widget

    @staticmethod
    def create_date_time_edit():
        date_time_edit = QDateTimeEdit()
        date_time_edit.setDateTime(QDateTime.currentDateTime())  # Default to current time
        date_time_edit.setDisplayFormat("MM/dd/yyyy h:mm AP")
        return date_time_edit

    @staticmethod
    def create_button(text):
        button = QPushButton(text)
        return button
