from unittest.mock import MagicMock
import sys
from PyQt5.QtWidgets import QApplication
from PyQt5.QtTest import QTest
from PyQt5.QtCore import Qt
from ngps_gui import NgpsGui  # Assuming the main GUI class is in ngps_gui.py


def test_gui():
    # Create the app and window
    app = QApplication(sys.argv)
    window = NgpsGui()

    # Create a mock ViewModel
    mock_view_model = MagicMock()
    window.view_model = mock_view_model

    # Set up the mock behaviors
    mock_view_model.check_ngps_status.return_value = True
    mock_view_model.fetch_ngps_status.return_value = "Mocked NGPS Status: All systems operational."
    mock_view_model.execute_camera_command.return_value = "Mocked Camera Command Executed."

    # Simulate user interaction: Test if the NGPS status is displayed correctly
    QTest.mouseClick(window.status_timer, Qt.LeftButton)  # Simulate a timer timeout (NGPS Status fetch)
    
    # Assert the expected results
    assert window.status_text_area.toPlainText() == "Mocked NGPS Status: All systems operational."

    # Test camera command execution
    mock_view_model.execute_camera_command.return_value = "Mocked Camera Command Executed."
    window.camera_dropdown.setCurrentIndex(2)  # Assuming "pause" is at index 2
    QTest.mouseClick(window.execute_button, Qt.LeftButton)  # Simulate clicking the execute button
    
    assert window.status_label.text() == "Mocked Camera Command Executed."

    # Test shutter state toggle
    mock_view_model.get_shutter_state.return_value = 'enabled'
    window.execute_camera_command()  # Simulate shutter enable command

    assert window.status_label.text() == "Shutter enabled."
    assert window.camera_dropdown.styleSheet() == "background-color: lightgreen;"

    # Run the application (this will open the GUI, but you may not need this during unit tests)
    window.show()
    sys.exit(app.exec_())


if __name__ == '__main__':
    test_gui()

