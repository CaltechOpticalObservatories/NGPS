import sys
import argparse
from PyQt5.QtCore import QTimer
from PyQt5.QtGui import QFont
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QHBoxLayout, QVBoxLayout

from zmq_service import (
    SeqguiZmqService, SeqguiZmqServiceThread,
    TOPIC_SEQ_SEQSTATE, TOPIC_SEQ_WAITSTATE, TOPIC_SEQ_DAEMONSTATE,
    TOPIC_ACAMD, TOPIC_SLICECAMD, TOPIC_BROADCAST, TOPIC_CAMERAD,
)
from panels import (
    StatePanel, SubsystemPanel, CameraPanel, AcquisitionPanel,
    WAIT_TCSOP, WAIT_USER,
    hline, vline, section_label,
)
from broadcast_log import BroadcastLog


BROKER_SUB_ENDPOINT = "tcp://localhost:5556"
BROKER_PUB_ENDPOINT = "tcp://localhost:5555"

BLINK_INTERVAL_MS = 500


class SeqguiMainWindow(QMainWindow):
    def __init__(self, sub_endpoint=BROKER_SUB_ENDPOINT, pub_endpoint=BROKER_PUB_ENDPOINT):
        super().__init__()
        self.setWindowTitle("Sequencer Monitor")
        self.resize(1000, 554)

        self.sub_endpoint = sub_endpoint
        self.pub_endpoint = pub_endpoint
        self.zmq_service = None
        self.zmq_service_thread = None

        # Dark theme applied globally via stylesheet
        self.setStyleSheet(
            "QMainWindow, QWidget { background-color: #1e1e1e; color: #e0e0e0; }"
            "QLabel { color: #e0e0e0; font-size: 10pt; }"
            "QPlainTextEdit {"
            "  background-color: #111111; color: #cccccc;"
            "  border: 1px solid #444;"
            "}"
        )

        # Initialize the UI
        self.init_ui()

        # Initialize services
        self.initialize_services()

        # Single shared blink timer drives all animated widgets
        self.blink_phase = False
        self.blink_timer = QTimer(self)
        self.blink_timer.setInterval(BLINK_INTERVAL_MS)
        self.blink_timer.timeout.connect(self._blink_tick)
        self.blink_timer.start()

    def init_ui(self):
        # Build panels
        self.state_panel  = StatePanel()
        self.subsys_panel = SubsystemPanel()
        self.camera_panel = CameraPanel()
        self.acq_panel    = AcquisitionPanel()
        self.log          = BroadcastLog()

        # Right column: subsystems / camera / acquisition stacked vertically
        right = QWidget()
        right_layout = QVBoxLayout(right)
        right_layout.setContentsMargins(0, 0, 0, 0)
        right_layout.setSpacing(0)
        right_layout.addWidget(self.subsys_panel, 0)
        right_layout.addWidget(hline())
        right_layout.addWidget(self.camera_panel, 0)
        right_layout.addWidget(hline())
        right_layout.addWidget(self.acq_panel, 0)
        right_layout.addStretch(1)

        # Top row: state on left, right column on right
        top = QWidget()
        top_layout = QHBoxLayout(top)
        top_layout.setContentsMargins(0, 0, 0, 0)
        top_layout.setSpacing(0)
        top_layout.addWidget(self.state_panel, 0)
        top_layout.addWidget(vline())
        top_layout.addWidget(right, 1)

        # Central layout: top status, message log
        central = QWidget()
        cl = QVBoxLayout(central)
        cl.setContentsMargins(4, 4, 4, 4)
        cl.setSpacing(0)
        cl.addWidget(top, 0)
        cl.addWidget(hline())
        cl.addSpacing(6)
        cl.addWidget(section_label("MESSAGES"))
        cl.addWidget(self.log, 1)
        self.setCentralWidget(central)

    def initialize_services(self):
        # Initialize the SeqguiZmqService
        self.zmq_service = SeqguiZmqService(self, self.sub_endpoint, self.pub_endpoint)
        self.zmq_service.connect()

        # Subscribe to the topics we care about
        for topic in (TOPIC_SEQ_SEQSTATE, TOPIC_SEQ_WAITSTATE, TOPIC_SEQ_DAEMONSTATE,
                      TOPIC_ACAMD, TOPIC_SLICECAMD, TOPIC_BROADCAST, TOPIC_CAMERAD):
            self.zmq_service.subscribe_to_topic(topic)

        # Connect the signals from SeqguiZmqService to the appropriate slots
        self.zmq_service.seqstate_changed.connect(self.state_panel.set_state)
        self.zmq_service.waitstate_changed.connect(self._on_waitstate)
        self.zmq_service.daemonstate_changed.connect(self.subsys_panel.set_daemonstate)
        self.zmq_service.acamd_changed.connect(self.acq_panel.set_acamd)
        self.zmq_service.slicecamd_changed.connect(self.acq_panel.set_slicecamd)
        self.zmq_service.camerad_changed.connect(self.camera_panel.set_camerad)
        self.zmq_service.sequencerd_alive.connect(self.subsys_panel.set_sequencerd_online)
        self.zmq_service.broadcast_received.connect(self.log.append)
        self.zmq_service.connection_error.connect(self._on_connection_error)

        # Start the SeqguiZmqService in a separate thread
        self.zmq_service_thread = SeqguiZmqServiceThread(self.zmq_service)
        self.zmq_service_thread.start()

    def _on_waitstate(self, state):
        """ Fan out a waitstate update to the panels that care about it. """
        self.subsys_panel.set_waitstate(state)
        self.state_panel.set_tcsop_active(bool(state.get(WAIT_TCSOP, False)))
        self.state_panel.set_user_active(bool(state.get(WAIT_USER, False)))

    def _on_connection_error(self, msg):
        """ Surface a ZMQ connection error as an ERROR row in the log. """
        self.log.append("ERROR", "seqgui", msg)

    def _blink_tick(self):
        """ Toggle the global blink phase and notify all animated panels. """
        self.blink_phase = not self.blink_phase
        self.state_panel.blink_tick(self.blink_phase)
        self.subsys_panel.blink_tick(self.blink_phase)
        self.camera_panel.blink_tick(self.blink_phase)
        self.acq_panel.blink_tick(self.blink_phase)

    def closeEvent(self, event):
        """ Stop the ZMQ worker thread cleanly on window close. """
        if self.zmq_service is not None:
            self.zmq_service.stop()
        if self.zmq_service_thread is not None:
            self.zmq_service_thread.wait(2000)
        super().closeEvent(event)


def main():
    parser = argparse.ArgumentParser(description="NGPS sequencer monitor GUI")
    parser.add_argument("--sub", default=BROKER_SUB_ENDPOINT,
                        help="ZMQ broker subscriber endpoint")
    parser.add_argument("--pub", default=BROKER_PUB_ENDPOINT,
                        help="ZMQ broker publisher endpoint")
    args = parser.parse_args()

    app = QApplication(sys.argv)
    app.setFont(QFont("Sans", 10))
    window = SeqguiMainWindow(args.sub, args.pub)
    window.show()
    return app.exec_()


if __name__ == "__main__":
    sys.exit(main())
