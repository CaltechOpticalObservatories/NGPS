"""
Record Editor Dialog for NGPS Database GUI

Provides full-featured edit dialog with NULL checkbox support for all columns.
Based on C++ RecordEditorDialog implementation.
"""

from typing import Dict, Any, List, Optional
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (
    QDialog, QVBoxLayout, QHBoxLayout, QFormLayout, QLabel,
    QLineEdit, QCheckBox, QPushButton, QMessageBox, QScrollArea,
    QWidget, QDialogButtonBox, QGroupBox
)

# Try to import normalization functions
try:
    from tools.ngps_db_gui import (
        ColumnMeta, normalize_target_row, normalize_channel_value,
        normalize_exptime_value, normalize_slitwidth_value
    )
    NORMALIZATION_AVAILABLE = True
except ImportError:
    NORMALIZATION_AVAILABLE = False

    # Fallback minimal ColumnMeta
    class ColumnMeta:
        def __init__(self, name, type, nullable, key, default, extra):
            self.name = name
            self.type = type
            self.nullable = nullable
            self.key = key
            self.default = default
            self.extra = extra

        @property
        def is_primary(self):
            return "PRI" in str(self.key)

        @property
        def is_auto_increment(self):
            return "auto_increment" in str(self.extra).lower()


class RecordEditorDialog(QDialog):
    """
    Full-featured record editor with NULL checkbox support.

    Features:
    - Edit all table columns
    - NULL checkbox for nullable columns
    - Automatic data normalization
    - Primary key and auto-increment handling
    - Scrollable form for many columns
    """

    def __init__(self, parent=None, columns: List[ColumnMeta] = None,
                 values: Dict[str, Any] = None, is_new: bool = False):
        """
        Initialize record editor dialog.

        Args:
            parent: Parent widget
            columns: List of ColumnMeta objects describing table schema
            values: Current row values (None for new record)
            is_new: True if creating new record, False if editing existing
        """
        super().__init__(parent)
        self.columns = columns or []
        self.values = values or {}
        self.is_new = is_new

        # Storage for field widgets
        self.field_widgets: Dict[str, QLineEdit] = {}
        self.null_checkboxes: Dict[str, QCheckBox] = {}

        # Setup UI
        self.setWindowTitle("New Record" if is_new else "Edit Record")
        self.setMinimumWidth(600)
        self.setMinimumHeight(400)

        self._init_ui()
        self._populate_fields()

    def _init_ui(self):
        """Initialize UI components."""
        layout = QVBoxLayout(self)

        # Create scrollable area for form
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)

        # Form widget
        form_widget = QWidget()
        form_layout = QFormLayout(form_widget)
        form_layout.setFieldGrowthPolicy(QFormLayout.ExpandingFieldsGrow)

        # Group columns by category
        key_columns = []
        required_columns = []
        optional_columns = []
        otm_columns = []

        for col in self.columns:
            if col.is_primary or col.is_auto_increment:
                key_columns.append(col)
            elif col.name.startswith("OTM"):
                otm_columns.append(col)
            elif not col.nullable:
                required_columns.append(col)
            else:
                optional_columns.append(col)

        # Add key fields (read-only for existing records)
        if key_columns:
            key_group = QGroupBox("Primary Keys")
            key_layout = QFormLayout()

            for col in key_columns:
                field = QLineEdit()
                field.setReadOnly(True if not self.is_new or col.is_auto_increment else False)
                if col.is_auto_increment:
                    field.setPlaceholderText("(auto-generated)")

                key_layout.addRow(f"{col.name}:", field)
                self.field_widgets[col.name] = field

            key_group.setLayout(key_layout)
            form_layout.addRow(key_group)

        # Add required fields
        if required_columns:
            req_group = QGroupBox("Required Fields")
            req_layout = QFormLayout()

            for col in required_columns:
                field = QLineEdit()
                field.setPlaceholderText(f"Required ({col.type})")

                req_layout.addRow(f"{col.name}*:", field)
                self.field_widgets[col.name] = field

            req_group.setLayout(req_layout)
            form_layout.addRow(req_group)

        # Add optional fields with NULL checkboxes
        if optional_columns:
            opt_group = QGroupBox("Optional Fields")
            opt_layout = QFormLayout()

            for col in optional_columns:
                # Create horizontal layout for field + NULL checkbox
                field_layout = QHBoxLayout()

                field = QLineEdit()
                field.setPlaceholderText(f"Optional ({col.type})")
                field_layout.addWidget(field, 1)

                null_check = QCheckBox("NULL")
                null_check.setToolTip("Check to set this field to NULL")
                null_check.stateChanged.connect(
                    lambda state, f=field: f.setEnabled(state != Qt.Checked)
                )
                field_layout.addWidget(null_check)

                opt_layout.addRow(f"{col.name}:", field_layout)

                self.field_widgets[col.name] = field
                self.null_checkboxes[col.name] = null_check

            opt_group.setLayout(opt_layout)
            form_layout.addRow(opt_group)

        # Add OTM fields (usually auto-populated, but editable)
        if otm_columns:
            otm_group = QGroupBox("OTM Results (Auto-populated)")
            otm_layout = QFormLayout()

            for col in otm_columns:
                # Create horizontal layout for field + NULL checkbox
                field_layout = QHBoxLayout()

                field = QLineEdit()
                field.setPlaceholderText(f"OTM field ({col.type})")
                field_layout.addWidget(field, 1)

                if col.nullable:
                    null_check = QCheckBox("NULL")
                    null_check.setToolTip("Check to set this field to NULL")
                    null_check.stateChanged.connect(
                        lambda state, f=field: f.setEnabled(state != Qt.Checked)
                    )
                    field_layout.addWidget(null_check)
                    self.null_checkboxes[col.name] = null_check

                otm_layout.addRow(f"{col.name}:", field_layout)
                self.field_widgets[col.name] = field

            otm_group.setLayout(otm_layout)
            form_layout.addRow(otm_group)

        scroll.setWidget(form_widget)
        layout.addWidget(scroll, 1)

        # Buttons
        button_box = QDialogButtonBox(
            QDialogButtonBox.Ok | QDialogButtonBox.Cancel | QDialogButtonBox.Apply
        )
        button_box.accepted.connect(self.accept)
        button_box.rejected.connect(self.reject)
        button_box.button(QDialogButtonBox.Apply).clicked.connect(self._apply_normalization)

        layout.addWidget(button_box)

    def _populate_fields(self):
        """Populate fields with current values."""
        for col_name, field in self.field_widgets.items():
            value = self.values.get(col_name, "")

            # Check if value is None/NULL
            is_null = value is None or str(value).strip().upper() == "NULL"

            if is_null and col_name in self.null_checkboxes:
                # Set NULL checkbox
                self.null_checkboxes[col_name].setChecked(True)
                field.clear()
                field.setEnabled(False)
            else:
                # Set field value
                field.setText(str(value) if value is not None else "")
                if col_name in self.null_checkboxes:
                    self.null_checkboxes[col_name].setChecked(False)

    def _apply_normalization(self):
        """Apply data normalization to fields."""
        if not NORMALIZATION_AVAILABLE:
            QMessageBox.information(
                self,
                "Normalization",
                "Data normalization is not available.\n\n"
                "Values will be used as-entered."
            )
            return

        try:
            # Get current values
            current_values = self.get_values()

            # Apply normalization
            result = normalize_target_row(current_values)

            if result.changed_columns:
                # Update fields with normalized values
                for col_name in result.changed_columns:
                    if col_name in self.field_widgets:
                        normalized_val = current_values.get(col_name, "")
                        self.field_widgets[col_name].setText(str(normalized_val))

                # Show message
                QMessageBox.information(
                    self,
                    "Normalization Applied",
                    f"Normalized fields:\n\n{', '.join(result.changed_columns)}\n\n{result.message}"
                )
            else:
                QMessageBox.information(
                    self,
                    "Normalization",
                    "All fields are already normalized."
                )

        except Exception as e:
            QMessageBox.warning(
                self,
                "Normalization Error",
                f"Error applying normalization:\n\n{e}"
            )

    def get_values(self) -> Dict[str, Any]:
        """
        Get current field values.

        Returns:
            Dictionary of column_name -> value (None for NULL)
        """
        values = {}

        for col_name, field in self.field_widgets.items():
            # Check if NULL checkbox is checked
            null_check = self.null_checkboxes.get(col_name)
            if null_check and null_check.isChecked():
                values[col_name] = None
            else:
                text = field.text().strip()
                values[col_name] = text if text else None

        return values

    def validate(self) -> tuple[bool, str]:
        """
        Validate field values.

        Returns:
            Tuple of (is_valid, error_message)
        """
        errors = []

        for col in self.columns:
            # Skip auto-increment columns
            if col.is_auto_increment:
                continue

            # Check required fields
            if not col.nullable:
                field = self.field_widgets.get(col.name)
                if field:
                    text = field.text().strip()
                    if not text:
                        errors.append(f"{col.name} is required")

        if errors:
            return False, "\n".join(errors)

        return True, ""

    def accept(self):
        """Validate and accept dialog."""
        # Validate fields
        is_valid, error_msg = self.validate()

        if not is_valid:
            QMessageBox.warning(
                self,
                "Validation Error",
                f"Please correct the following errors:\n\n{error_msg}"
            )
            return

        super().accept()


def edit_record_dialog(parent, columns: List[ColumnMeta], values: Dict[str, Any]) -> Optional[Dict[str, Any]]:
    """
    Convenience function to show edit dialog and get result.

    Args:
        parent: Parent widget
        columns: Table column metadata
        values: Current row values

    Returns:
        Updated values if accepted, None if cancelled
    """
    dialog = RecordEditorDialog(parent, columns, values, is_new=False)

    if dialog.exec_() == QDialog.Accepted:
        return dialog.get_values()

    return None


def add_record_dialog(parent, columns: List[ColumnMeta]) -> Optional[Dict[str, Any]]:
    """
    Convenience function to show add dialog and get result.

    Args:
        parent: Parent widget
        columns: Table column metadata

    Returns:
        New record values if accepted, None if cancelled
    """
    dialog = RecordEditorDialog(parent, columns, {}, is_new=True)

    if dialog.exec_() == QDialog.Accepted:
        return dialog.get_values()

    return None
