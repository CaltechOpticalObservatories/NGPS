import sys
import argparse
from PyQt5.QtCore import QTimer
from PyQt5.QtGui import QFont
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QGroupBox, QHBoxLayout, QVBoxLayout
)

from zmq_service import (
    SeqguiZmqService, SeqguiZmqServiceThread,
    TOPIC_SEQ_SEQSTATE, TOPIC_SEQ_WAITSTATE, TOPIC_SEQ_DAEMONSTATE,
    TOPIC_ACAMD, TOPIC_SLICECAMD, TOPIC_BROADCAST, TOPIC_CAMERAD,
)
from panels import (
    StatePanel, SubsystemPanel, CameraPanel, AcquisitionPanel,
    WAIT_TCSOP, WAIT_USER,
)
from broadcast_log import BroadcastLog
from udp_service import (
    SeqguiUdpService, SeqguiUdpServiceThread, MULTICAST_GROUP, MULTICAST_PORT,
)


BROKER_SUB_ENDPOINT = "tcp://localhost:5556"
BROKER_PUB_ENDPOINT = "tcp://localhost:5555"

BLINK_INTERVAL_MS = 500


class SeqguiMainWindow(QMainWindow):
    def __init__(self, sub_endpoint=BROKER_SUB_ENDPOINT, pub_endpoint=BROKER_PUB_ENDPOINT,
                 udp_group=MULTICAST_GROUP, udp_port=MULTICAST_PORT):
        super().__init__()
        self.setWindowTitle("Sequencer Monitor")

        self.sub_endpoint = sub_endpoint
        self.pub_endpoint = pub_endpoint
        self.udp_group = udp_group
        self.udp_port = udp_port
        self.zmq_service = None
        self.zmq_service_thread = None
        self.udp_service = None
        self.udp_service_thread = None

        # Dark theme applied globally via stylesheet
        self.setStyleSheet(
            "QMainWindow, QWidget { background-color: #1e1e1e; color: #e0e0e0; }"
            "QLabel { color: #e0e0e0; font-size: 10pt; }"
            "QPlainTextEdit {"
            "  background-color: #111111; color: #cccccc;"
            "  border: 1px solid #444;"
            "}"
            "QGroupBox {"
            "  border: 1px solid #3a3a3a; border-radius: 6px;"
            "  margin-top: 14px; padding-top: 2px;"
            "}"
            "QGroupBox::title {"
            "  subcontrol-origin: margin; subcontrol-position: top left;"
            "  left: 10px; padding: 0 4px;"
            "  color: #aaaaaa; font-weight: bold; font-size: 10pt;"
            "}"
        )

        # Initialize the UI
        self.init_ui()

        # Open at a fixed, non-resizable size (the status layout's minimum).
        self.centralWidget().layout().activate()
        self.setFixedSize(self.minimumSizeHint().width(), 554)

        # Initialize services
        self.initialize_services()
        self.initialize_udp_service()

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
        right_layout.setSpacing(6)
        right_layout.addWidget(self.subsys_panel, 0)
        right_layout.addWidget(self.camera_panel, 0)
        right_layout.addWidget(self.acq_panel, 0)
        right_layout.addStretch(1)

        # Top row: state on left, right column on right
        top = QWidget()
        top_layout = QHBoxLayout(top)
        top_layout.setContentsMargins(0, 0, 0, 0)
        top_layout.setSpacing(6)
        top_layout.addWidget(self.state_panel, 0)
        top_layout.addWidget(right, 1)

        # MESSAGES card wrapping the broadcast log
        msg_box = QGroupBox("MESSAGES")
        msg_layout = QVBoxLayout(msg_box)
        msg_layout.setContentsMargins(6, 4, 6, 6)
        msg_layout.addWidget(self.log)

        # Central layout: top status, message log
        central = QWidget()
        cl = QVBoxLayout(central)
        cl.setContentsMargins(6, 6, 6, 6)
        cl.setSpacing(6)
        cl.addWidget(top, 0)
        cl.addWidget(msg_box, 1)
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
        self.zmq_service.daemonstate_changed.connect(self._on_daemonstate)
        self.zmq_service.acamd_changed.connect(self.acq_panel.set_acamd)
        self.zmq_service.slicecamd_changed.connect(self.acq_panel.set_slicecamd)
        self.zmq_service.camerad_changed.connect(self.camera_panel.set_camerad)
        self.zmq_service.sequencerd_alive.connect(self.subsys_panel.set_sequencerd_online)
        self.zmq_service.broadcast_received.connect(self.log.append)
        self.zmq_service.connection_error.connect(self._on_connection_error)

        # Start the SeqguiZmqService in a separate thread
        self.zmq_service_thread = SeqguiZmqServiceThread(self.zmq_service)
        self.zmq_service_thread.start()

    def initialize_udp_service(self):
        """ Start the UDP multicast listener that drives the camera progress
            bars. Failure to open the socket is non-fatal -- the rest of the
            GUI still runs, just without progress updates. """
        self.udp_service = SeqguiUdpService(self.udp_group, self.udp_port)
        try:
            self.udp_service.connect()
        except OSError as e:
            self.log.append("ERROR", "seqgui", f"UDP listener failed: {e}")
            self.udp_service = None
            return
        self.udp_service.exposure_progress.connect(self.camera_panel.set_exposure_progress)
        self.udp_service.readout_progress.connect(self.camera_panel.set_readout_progress)
        self.udp_service.connection_error.connect(self._on_connection_error)

        self.udp_service_thread = SeqguiUdpServiceThread(self.udp_service)
        self.udp_service_thread.start()

    def _on_waitstate(self, state):
        """ Fan out a waitstate update to the panels that care about it. """
        self.subsys_panel.set_waitstate(state)
        self.state_panel.set_tcsop_active(bool(state.get(WAIT_TCSOP, False)))
        self.state_panel.set_user_active(bool(state.get(WAIT_USER, False)))

    def _on_daemonstate(self, state):
        """ Fan out a daemonstate update; clear stale indicators on dependent
            panels when their owning daemon goes DOWN, otherwise the last
            published acam/slicecam/camera state would stick on the UI. """
        self.subsys_panel.set_daemonstate(state)
        self.camera_panel.set_camerad_online(bool(state.get("camerad", False)))
        self.acq_panel.set_acamd_online(bool(state.get("acamd", False)))
        self.acq_panel.set_slicecamd_online(bool(state.get("slicecamd", False)))

    def _on_connection_error(self, msg):
        """ Surface a ZMQ connection error as an ERROR row in the log. """
        self.log.append("ERROR", "seqgui", msg)

    def _blink_tick(self):
        """ Toggle the global blink phase and notify all animated panels. """
        self.blink_phase = not self.blink_phase
        self.state_panel.blink_tick(self.blink_phase)
        self.subsys_panel.blink_tick(self.blink_phase)
        self.acq_panel.blink_tick(self.blink_phase)

    def closeEvent(self, event):
        """ Stop the ZMQ worker thread cleanly on window close. """
        if self.zmq_service is not None:
            self.zmq_service.stop()
        if self.zmq_service_thread is not None:
            self.zmq_service_thread.wait(2000)
        if self.udp_service is not None:
            self.udp_service.stop()
        if self.udp_service_thread is not None:
            self.udp_service_thread.wait(2000)
        super().closeEvent(event)


def main():
    parser = argparse.ArgumentParser(description="NGPS sequencer monitor GUI")
    parser.add_argument("--sub", default=BROKER_SUB_ENDPOINT,
                        help="ZMQ broker subscriber endpoint")
    parser.add_argument("--pub", default=BROKER_PUB_ENDPOINT,
                        help="ZMQ broker publisher endpoint")
    parser.add_argument("--group", default=MULTICAST_GROUP,
                        help="UDP multicast group for camera progress")
    parser.add_argument("--port", type=int, default=MULTICAST_PORT,
                        help="UDP multicast port for camera progress")
    args = parser.parse_args()

    app = QApplication(sys.argv)
    app.setFont(QFont("Sans", 10))
    window = SeqguiMainWindow(args.sub, args.pub, args.group, args.port)
    window.show()
    return app.exec_()


if __name__ == "__main__":
    sys.exit(main())
