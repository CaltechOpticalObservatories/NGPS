import math
import socket
import struct
import time
from PyQt5.QtCore import pyqtSignal, QObject, QThread

# camerad async broadcasts go to this multicast group/port (MESSAGEGROUP /
# MESSAGEPORT in the daemon .cfg files; utils/listener.cpp is the C++ reference).
MULTICAST_GROUP = "239.1.1.234"
MULTICAST_PORT  = 1300

# Async message prefixes we care about (see camerad/astrocam.cpp):
#   EXPTIME:<remain_ms> <total_ms> <percent>          Interface::exposure_progress
#   PIXELCOUNT_<dev>:<pixelcount> <imagesize> <pct>   Callback::readCallback
PREFIX_EXPTIME    = "EXPTIME:"
PREFIX_PIXELCOUNT = "PIXELCOUNT_"


def _clamp_pct(value):
    """ Clamp a numeric percent to the integer range 0-100. """
    return max(0, min(100, int(round(value))))


class SeqguiUdpService(QObject):
    """ Listens to the daemon UDP multicast stream and turns camerad's
        EXPTIME/PIXELCOUNT async messages into exposure/readout progress
        percentages. """

    exposure_progress = pyqtSignal(int, int)   # percent, seconds remaining
    readout_progress  = pyqtSignal(int, int)   # percent, ETA seconds (-1 unknown)
    connection_error  = pyqtSignal(str)

    def __init__(self, group=MULTICAST_GROUP, port=MULTICAST_PORT):
        super().__init__()
        self.group = group
        self.port = port
        self.sock = None
        self.running = True
        self._readout = {}             # devnum -> percent (0-100)
        self._readout_rate = {}        # devnum -> (time, pixels, ema_rate, imagesize)

    def connect(self):
        """ Open and join the UDP multicast group (mirrors utils/listener.cpp). """
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(("", self.port))
        mreq = struct.pack("4sl", socket.inet_aton(self.group), socket.INADDR_ANY)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        self.sock.settimeout(0.5)      # so the loop can poll self.running

    def stop(self):
        """ Request a clean shutdown of the listen loop. """
        self.running = False

    def listen(self):
        """ Receive datagrams and dispatch recognized progress messages. """
        if self.sock is None:
            return
        while self.running:
            try:
                data, _addr = self.sock.recvfrom(1024)
            except socket.timeout:
                continue
            except OSError:
                break
            for line in data.decode("utf-8", errors="replace").splitlines():
                self._dispatch(line.strip())
        self._close()

    def _dispatch(self, line):
        """ Route one async message line to the matching progress parser. """
        if line.startswith(PREFIX_EXPTIME):
            self._parse_exposure(line)
        elif line.startswith(PREFIX_PIXELCOUNT):
            self._parse_readout(line)

    def _parse_exposure(self, line):
        """ EXPTIME:<remain_ms> <total_ms> <percent>. The bar uses the
            precomputed percent; the label shows whole seconds remaining. """
        try:
            _head, _, rest = line.partition(":")
            fields = rest.split()
            remain_ms = int(fields[0])
            percent   = int(fields[2])
        except (ValueError, IndexError):
            return
        seconds = max(0, math.ceil(remain_ms / 1000))
        self.exposure_progress.emit(_clamp_pct(percent), seconds)

    def _parse_readout(self, line):
        """ PIXELCOUNT_<dev>:<pixelcount> <imagesize> <percent>. The bar tracks
            the slowest (min-percent) device; the label shows an ETA estimated
            from each device's smoothed pixel rate (last device to finish wins).
            Percent fills the bar immediately; the ETA needs two samples. """
        try:
            head, _, rest = line.partition(":")
            devnum = int(head[len(PREFIX_PIXELCOUNT):])
            fields = rest.split()
            pixels    = int(fields[0])
            imagesize = int(fields[1])
            percent   = int(fields[2])
        except (ValueError, IndexError):
            return
        # clear stale state once a prior readout has fully completed
        if self._readout and all(v >= 100 for v in self._readout.values()):
            self._readout = {}
            self._readout_rate = {}

        now = time.monotonic()
        self._readout[devnum] = percent

        # update this device's smoothed (EMA) pixel rate from the prior sample
        prev = self._readout_rate.get(devnum)
        ema = prev[2] if prev else None
        if prev:
            dt = now - prev[0]
            dpix = pixels - prev[1]
            if dt > 0 and dpix > 0:
                rate = dpix / dt
                ema = rate if prev[2] is None else 0.3 * rate + 0.7 * prev[2]
        self._readout_rate[devnum] = (now, pixels, ema, imagesize)

        # ETA = time for the slowest-finishing device to complete (max over devs)
        etas = [(size - pix) / r
                for (_, pix, r, size) in self._readout_rate.values()
                if r and r > 0 and pix < size]
        eta = math.ceil(max(etas)) if etas else -1

        self.readout_progress.emit(_clamp_pct(min(self._readout.values())), eta)

    def _close(self):
        if self.sock is not None:
            try:
                self.sock.close()
            except OSError:
                pass
            self.sock = None


class SeqguiUdpServiceThread(QThread):
    def __init__(self, udp_service):
        super().__init__()
        self.udp_service = udp_service

    def run(self):
        self.udp_service.listen()
