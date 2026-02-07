"""
Bulk Operations Dialog for NGPS Database GUI

Provides bulk editing, column visibility management, filtering, and CSV operations.
"""

from typing import Dict, Any, List, Optional, Set
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (
    QDialog, QVBoxLayout, QHBoxLayout, QFormLayout, QLabel,
    QLineEdit, QCheckBox, QPushButton, QMessageBox, QComboBox,
    QDialogButtonBox, QGroupBox, QListWidget, QListWidgetItem,
    QFileDialog, QTextEdit, QRadioButton, QButtonGroup
)


class BulkEditDialog(QDialog):
    """
    Dialog for bulk editing a column value across multiple rows.

    Features:
    - Set column to specific value
    - Set column to NULL
    - Append/prepend text
    - Mathematical operations (for numeric columns)
    """

    def __init__(self, parent=None, column_name: str = "",
                 column_type: str = "", num_rows: int = 0):
        """
        Initialize bulk edit dialog.

        Args:
            parent: Parent widget
            column_name: Name of column to edit
            column_type: SQL type of column
            num_rows: Number of rows that will be affected
        """
        super().__init__(parent)

        self.column_name = column_name
        self.column_type = column_type
        self.num_rows = num_rows

        self.setWindowTitle(f"Bulk Edit - {column_name}")
        self.setMinimumWidth(500)

        self._init_ui()

    def _init_ui(self):
        """Initialize UI components."""
        layout = QVBoxLayout(self)

        # Info label
        info_label = QLabel(
            f"Editing column: <b>{self.column_name}</b> ({self.column_type})<br>"
            f"Rows affected: <b>{self.num_rows}</b>"
        )
        layout.addWidget(info_label)

        # Operation group
        operation_group = QGroupBox("Operation")
        operation_layout = QVBoxLayout()

        self.operation_group = QButtonGroup(self)

        # Set value option
        self.set_value_radio = QRadioButton("Set to value:")
        self.set_value_radio.setChecked(True)
        self.operation_group.addButton(self.set_value_radio, 0)
        operation_layout.addWidget(self.set_value_radio)

        self.value_input = QLineEdit()
        self.value_input.setPlaceholderText("Enter new value")
        operation_layout.addWidget(self.value_input)

        # Set NULL option
        self.set_null_radio = QRadioButton("Set to NULL")
        self.operation_group.addButton(self.set_null_radio, 1)
        operation_layout.addWidget(self.set_null_radio)

        # Append option (for string columns)
        if "char" in self.column_type.lower() or "text" in self.column_type.lower():
            operation_layout.addWidget(QLabel(""))  # Spacer

            self.append_radio = QRadioButton("Append text:")
            self.operation_group.addButton(self.append_radio, 2)
            operation_layout.addWidget(self.append_radio)

            self.append_input = QLineEdit()
            self.append_input.setPlaceholderText("Text to append")
            operation_layout.addWidget(self.append_input)

            self.prepend_radio = QRadioButton("Prepend text:")
            self.operation_group.addButton(self.prepend_radio, 3)
            operation_layout.addWidget(self.prepend_radio)

            self.prepend_input = QLineEdit()
            self.prepend_input.setPlaceholderText("Text to prepend")
            operation_layout.addWidget(self.prepend_input)

        # Mathematical operations (for numeric columns)
        if any(t in self.column_type.lower() for t in ["int", "float", "double", "decimal"]):
            operation_layout.addWidget(QLabel(""))  # Spacer

            self.add_radio = QRadioButton("Add value:")
            self.operation_group.addButton(self.add_radio, 4)
            operation_layout.addWidget(self.add_radio)

            self.add_input = QLineEdit()
            self.add_input.setPlaceholderText("Value to add")
            operation_layout.addWidget(self.add_input)

            self.multiply_radio = QRadioButton("Multiply by:")
            self.operation_group.addButton(self.multiply_radio, 5)
            operation_layout.addWidget(self.multiply_radio)

            self.multiply_input = QLineEdit()
            self.multiply_input.setPlaceholderText("Multiplier")
            operation_layout.addWidget(self.multiply_input)

        operation_group.setLayout(operation_layout)
        layout.addWidget(operation_group)

        # Dialog buttons
        button_box = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        button_box.accepted.connect(self.accept)
        button_box.rejected.connect(self.reject)
        layout.addWidget(button_box)

    def get_operation(self) -> Dict[str, Any]:
        """
        Get the selected operation.

        Returns:
            Dictionary with 'type' and 'value' keys describing the operation
        """
        operation_id = self.operation_group.checkedId()

        if operation_id == 0:  # Set value
            return {"type": "set", "value": self.value_input.text()}
        elif operation_id == 1:  # Set NULL
            return {"type": "null", "value": None}
        elif operation_id == 2:  # Append
            return {"type": "append", "value": self.append_input.text()}
        elif operation_id == 3:  # Prepend
            return {"type": "prepend", "value": self.prepend_input.text()}
        elif operation_id == 4:  # Add
            return {"type": "add", "value": self.add_input.text()}
        elif operation_id == 5:  # Multiply
            return {"type": "multiply", "value": self.multiply_input.text()}
        else:
            return {"type": "set", "value": ""}


class ColumnVisibilityDialog(QDialog):
    """
    Dialog for managing column visibility.

    Allows users to show/hide columns in the table view.
    """

    def __init__(self, parent=None, columns: List[str] = None,
                 visible_columns: Set[str] = None):
        """
        Initialize column visibility dialog.

        Args:
            parent: Parent widget
            columns: List of all column names
            visible_columns: Set of currently visible column names
        """
        super().__init__(parent)

        self.columns = columns or []
        self.visible_columns = visible_columns or set(self.columns)
        self.initial_visible = self.visible_columns.copy()

        self.setWindowTitle("Column Visibility")
        self.setMinimumSize(400, 500)

        self._init_ui()

    def _init_ui(self):
        """Initialize UI components."""
        layout = QVBoxLayout(self)

        # Info label
        info_label = QLabel(
            "Select which columns to display in the table.\n"
            "Primary key columns are always visible."
        )
        layout.addWidget(info_label)

        # Column list
        self.column_list = QListWidget()

        for col in self.columns:
            item = QListWidgetItem(col)
            item.setFlags(item.flags() | Qt.ItemIsUserCheckable)

            # Check if column is currently visible
            if col in self.visible_columns:
                item.setCheckState(Qt.Checked)
            else:
                item.setCheckState(Qt.Unchecked)

            self.column_list.addItem(item)

        layout.addWidget(self.column_list)

        # Preset buttons
        preset_layout = QHBoxLayout()

        show_all_btn = QPushButton("Show All")
        show_all_btn.clicked.connect(self._show_all)
        preset_layout.addWidget(show_all_btn)

        hide_all_btn = QPushButton("Hide All")
        hide_all_btn.clicked.connect(self._hide_all)
        preset_layout.addWidget(hide_all_btn)

        show_essential_btn = QPushButton("Essential Only")
        show_essential_btn.setToolTip("Show only essential columns")
        show_essential_btn.clicked.connect(self._show_essential)
        preset_layout.addWidget(show_essential_btn)

        layout.addLayout(preset_layout)

        # Dialog buttons
        button_box = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        button_box.accepted.connect(self.accept)
        button_box.rejected.connect(self.reject)
        layout.addWidget(button_box)

    def _show_all(self):
        """Check all columns."""
        for i in range(self.column_list.count()):
            item = self.column_list.item(i)
            item.setCheckState(Qt.Checked)

    def _hide_all(self):
        """Uncheck all columns."""
        for i in range(self.column_list.count()):
            item = self.column_list.item(i)
            item.setCheckState(Qt.Unchecked)

    def _show_essential(self):
        """Show only essential columns."""
        essential = {
            "OBSERVATION_ID", "OBS_ORDER", "NAME", "RA", "DEC",
            "CHANNEL", "EXPTIME", "PRIORITY", "ENABLED"
        }

        for i in range(self.column_list.count()):
            item = self.column_list.item(i)
            col_name = item.text()

            if col_name in essential:
                item.setCheckState(Qt.Checked)
            else:
                item.setCheckState(Qt.Unchecked)

    def get_visible_columns(self) -> Set[str]:
        """
        Get the set of columns that should be visible.

        Returns:
            Set of column names
        """
        visible = set()

        for i in range(self.column_list.count()):
            item = self.column_list.item(i)
            if item.checkState() == Qt.Checked:
                visible.add(item.text())

        return visible


class AdvancedFilterDialog(QDialog):
    """
    Dialog for advanced filtering of table rows.

    Supports multiple filter conditions with AND/OR logic.
    """

    def __init__(self, parent=None, columns: List[str] = None):
        """
        Initialize advanced filter dialog.

        Args:
            parent: Parent widget
            columns: List of column names available for filtering
        """
        super().__init__(parent)

        self.columns = columns or []
        self.filters: List[Dict[str, Any]] = []

        self.setWindowTitle("Advanced Filter")
        self.setMinimumSize(600, 400)

        self._init_ui()

    def _init_ui(self):
        """Initialize UI components."""
        layout = QVBoxLayout(self)

        # Info label
        info_label = QLabel("Build a filter expression to show only matching rows.")
        layout.addWidget(info_label)

        # Filter builder
        filter_group = QGroupBox("Filter Conditions")
        filter_layout = QVBoxLayout()

        # Column selection
        col_layout = QHBoxLayout()
        col_layout.addWidget(QLabel("Column:"))
        self.column_combo = QComboBox()
        self.column_combo.addItems(self.columns)
        col_layout.addWidget(self.column_combo, 1)
        filter_layout.addLayout(col_layout)

        # Operator selection
        op_layout = QHBoxLayout()
        op_layout.addWidget(QLabel("Operator:"))
        self.operator_combo = QComboBox()
        self.operator_combo.addItems([
            "equals", "not equals", "contains", "starts with", "ends with",
            "greater than", "less than", "is NULL", "is not NULL"
        ])
        op_layout.addWidget(self.operator_combo, 1)
        filter_layout.addLayout(op_layout)

        # Value input
        val_layout = QHBoxLayout()
        val_layout.addWidget(QLabel("Value:"))
        self.value_input = QLineEdit()
        self.value_input.setPlaceholderText("Filter value")
        val_layout.addWidget(self.value_input, 1)
        filter_layout.addLayout(val_layout)

        # Add filter button
        add_filter_btn = QPushButton("Add Condition")
        add_filter_btn.clicked.connect(self._add_filter)
        filter_layout.addWidget(add_filter_btn)

        # Current filters display
        filter_layout.addWidget(QLabel("Active Filters:"))
        self.filters_display = QTextEdit()
        self.filters_display.setReadOnly(True)
        self.filters_display.setMaximumHeight(150)
        filter_layout.addWidget(self.filters_display)

        # Clear filters button
        clear_btn = QPushButton("Clear All Filters")
        clear_btn.clicked.connect(self._clear_filters)
        filter_layout.addWidget(clear_btn)

        filter_group.setLayout(filter_layout)
        layout.addWidget(filter_group)

        # Dialog buttons
        button_box = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        button_box.accepted.connect(self.accept)
        button_box.rejected.connect(self.reject)
        layout.addWidget(button_box)

    def _add_filter(self):
        """Add a filter condition."""
        column = self.column_combo.currentText()
        operator = self.operator_combo.currentText()
        value = self.value_input.text()

        if not column:
            return

        # For NULL operators, value is not needed
        if operator not in ["is NULL", "is not NULL"] and not value:
            QMessageBox.warning(
                self,
                "Missing Value",
                "Please enter a filter value."
            )
            return

        self.filters.append({
            "column": column,
            "operator": operator,
            "value": value
        })

        self._update_filters_display()
        self.value_input.clear()

    def _clear_filters(self):
        """Clear all filter conditions."""
        self.filters.clear()
        self._update_filters_display()

    def _update_filters_display(self):
        """Update the filters display text."""
        if not self.filters:
            self.filters_display.setText("(no filters)")
            return

        text_lines = []
        for i, f in enumerate(self.filters, 1):
            if f["operator"] in ["is NULL", "is not NULL"]:
                text_lines.append(f"{i}. {f['column']} {f['operator']}")
            else:
                text_lines.append(f"{i}. {f['column']} {f['operator']} '{f['value']}'")

        self.filters_display.setText("\n".join(text_lines))

    def get_filters(self) -> List[Dict[str, Any]]:
        """
        Get the list of filter conditions.

        Returns:
            List of filter dictionaries
        """
        return self.filters


def export_to_csv(file_path: str, columns: List[str], rows: List[List[Any]]) -> bool:
    """
    Export table data to CSV file.

    Args:
        file_path: Path to save CSV file
        columns: List of column names
        rows: List of row data (each row is a list of values)

    Returns:
        True if successful, False otherwise
    """
    try:
        import csv

        with open(file_path, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)

            # Write header
            writer.writerow(columns)

            # Write rows
            for row in rows:
                writer.writerow(row)

        return True

    except Exception as e:
        print(f"Error exporting CSV: {e}")
        return False


def import_from_csv(file_path: str) -> Optional[Tuple[List[str], List[List[Any]]]]:
    """
    Import table data from CSV file.

    Args:
        file_path: Path to CSV file

    Returns:
        Tuple of (columns, rows) if successful, None otherwise
    """
    try:
        import csv

        columns = []
        rows = []

        with open(file_path, 'r', encoding='utf-8') as f:
            reader = csv.reader(f)

            # Read header
            columns = next(reader)

            # Read rows
            for row in reader:
                rows.append(row)

        return columns, rows

    except Exception as e:
        print(f"Error importing CSV: {e}")
        return None
