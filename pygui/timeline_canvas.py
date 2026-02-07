"""
Timeline Visualization Canvas for NGPS Database GUI

Custom QPainter-based widget for rendering observation schedules.

Features:
- Time axis with twilight markers
- Exposure bars with color coding
- Airmass curves overlay
- Interactive selection and tooltips
- Idle gap visualization

Based on C++ TimelineCanvas implementation.
"""

from typing import List, Dict, Optional, Tuple
from datetime import datetime
from PyQt5.QtCore import Qt, QRectF, QPointF, pyqtSignal, QTimer
from PyQt5.QtWidgets import QWidget, QToolTip
from PyQt5.QtGui import (
    QPainter, QPen, QBrush, QColor, QFont, QPainterPath,
    QLinearGradient, QMouseEvent, QPaintEvent
)

try:
    from otm_integration import TimelineData, TimelineTarget, otm_flag_color
    TIMELINE_DATA_AVAILABLE = True
except ImportError:
    TIMELINE_DATA_AVAILABLE = False
    TimelineData = None
    TimelineTarget = None


# Layout constants
LEFT_MARGIN = 200  # Space for target labels
RIGHT_MARGIN = 172  # Space for flags
ROW_HEIGHT = 26  # Base row height
SELECTED_ROW_EXTRA = 70  # Extra height for selected row (airmass curve)
TIME_AXIS_HEIGHT = 30  # Top and bottom time axis

# Colors
TWILIGHT_COLOR = QColor(180, 180, 180, 100)
IDLE_COLOR = QColor(220, 50, 50, 40)
GRID_COLOR = QColor(200, 200, 200)
SELECTED_BG_COLOR = QColor(240, 248, 255)

# Target colors (cycling)
TARGET_COLORS = [
    QColor(100, 149, 237),  # Cornflower blue
    QColor(60, 179, 113),   # Medium sea green
    QColor(238, 130, 238),  # Violet
    QColor(255, 165, 0),    # Orange
    QColor(220, 20, 60),    # Crimson
    QColor(64, 224, 208),   # Turquoise
]


class TimelineCanvas(QWidget):
    """
    Custom widget for rendering observation timeline.

    Signals:
        target_selected(str): Emitted when target is selected (OBSERVATION_ID)
        flag_clicked(str, str): Emitted when flag is clicked (OBS_ID, flag_text)
    """

    target_selected = pyqtSignal(str)
    flag_clicked = pyqtSignal(str, str)

    def __init__(self, parent=None):
        super().__init__(parent)

        self._timeline_data: Optional[TimelineData] = None
        self._selected_obs_id: str = ""
        self._hover_row: int = -1

        # Enable mouse tracking for hover effects
        self.setMouseTracking(True)

        # Minimum size
        self.setMinimumHeight(400)
        self.setMinimumWidth(800)

    def set_data(self, data: TimelineData) -> None:
        """Set timeline data and trigger repaint."""
        self._timeline_data = data
        self._update_size()
        self.update()

    def set_selected_obs_id(self, obs_id: str) -> None:
        """Set selected observation ID."""
        self._selected_obs_id = obs_id
        self.update()

    def _update_size(self) -> None:
        """Update widget size based on data."""
        if not self._timeline_data:
            return

        num_rows = len(self._timeline_data.targets)
        selected_extra = SELECTED_ROW_EXTRA if self._selected_obs_id else 0

        total_height = (
            TIME_AXIS_HEIGHT +  # Top axis
            num_rows * ROW_HEIGHT +
            selected_extra +
            TIME_AXIS_HEIGHT  # Bottom axis
        )

        self.setMinimumHeight(total_height)

    def paintEvent(self, event: QPaintEvent) -> None:
        """Paint timeline visualization."""
        if not self._timeline_data or not self._timeline_data.targets:
            return

        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        # Calculate layout
        plot_width = self.width() - LEFT_MARGIN - RIGHT_MARGIN
        plot_x = LEFT_MARGIN

        # Parse time range
        try:
            time_range = self._compute_time_range()
            if not time_range:
                return

            start_time, end_time = time_range

            # Draw components
            self._draw_background(painter)
            self._draw_twilight_markers(painter, plot_x, plot_width, start_time, end_time)
            self._draw_time_axis(painter, plot_x, plot_width, start_time, end_time)
            self._draw_idle_gaps(painter, plot_x, plot_width, start_time, end_time)
            self._draw_targets(painter, plot_x, plot_width, start_time, end_time)
            self._draw_flags(painter)

        except Exception as e:
            # Fallback: draw error message
            painter.setPen(Qt.red)
            painter.drawText(self.rect(), Qt.AlignCenter, f"Error rendering timeline: {e}")

    def _compute_time_range(self) -> Optional[Tuple[datetime, datetime]]:
        """Compute time range from timeline data."""
        if not self._timeline_data or not self._timeline_data.targets:
            return None

        # Get earliest and latest times from targets
        start_times = []
        end_times = []

        for target in self._timeline_data.targets:
            try:
                if target.start:
                    start_times.append(datetime.fromisoformat(target.start.replace('Z', '+00:00')))
                if target.end:
                    end_times.append(datetime.fromisoformat(target.end.replace('Z', '+00:00')))
            except (ValueError, AttributeError):
                continue

        if not start_times or not end_times:
            return None

        start_time = min(start_times)
        end_time = max(end_times)

        # Add padding (10% on each side)
        duration = (end_time - start_time).total_seconds()
        padding = duration * 0.1

        from datetime import timedelta
        start_time = start_time - timedelta(seconds=padding)
        end_time = end_time + timedelta(seconds=padding)

        return start_time, end_time

    def _time_to_x(self, time_str: str, plot_x: float, plot_width: float,
                   start_time: datetime, end_time: datetime) -> float:
        """Convert ISO timestamp to X coordinate."""
        try:
            t = datetime.fromisoformat(time_str.replace('Z', '+00:00'))
            duration = (end_time - start_time).total_seconds()
            if duration <= 0:
                return plot_x

            offset = (t - start_time).total_seconds()
            fraction = offset / duration
            return plot_x + fraction * plot_width
        except (ValueError, AttributeError):
            return plot_x

    def _draw_background(self, painter: QPainter) -> None:
        """Draw background."""
        painter.fillRect(self.rect(), Qt.white)

    def _draw_twilight_markers(self, painter: QPainter, plot_x: float, plot_width: float,
                                start_time: datetime, end_time: datetime) -> None:
        """Draw twilight markers."""
        if not self._timeline_data:
            return

        painter.setPen(QPen(TWILIGHT_COLOR, 1, Qt.DashLine))

        twilight_times = [
            ("Evening -16°", self._timeline_data.twilight_evening_16),
            ("Evening -12°", self._timeline_data.twilight_evening_12),
            ("Morning -12°", self._timeline_data.twilight_morning_12),
            ("Morning -16°", self._timeline_data.twilight_morning_16),
        ]

        for label, time_str in twilight_times:
            if not time_str:
                continue

            x = self._time_to_x(time_str, plot_x, plot_width, start_time, end_time)

            # Draw vertical line
            painter.drawLine(int(x), TIME_AXIS_HEIGHT, int(x), self.height() - TIME_AXIS_HEIGHT)

            # Draw label
            painter.setPen(Qt.gray)
            font = QFont()
            font.setPointSize(8)
            painter.setFont(font)
            painter.drawText(int(x) + 2, TIME_AXIS_HEIGHT - 5, label)
            painter.setPen(QPen(TWILIGHT_COLOR, 1, Qt.DashLine))

    def _draw_time_axis(self, painter: QPainter, plot_x: float, plot_width: float,
                        start_time: datetime, end_time: datetime) -> None:
        """Draw time axis with ticks and labels."""
        painter.setPen(Qt.black)

        # Draw axis lines
        y_top = TIME_AXIS_HEIGHT
        y_bottom = self.height() - TIME_AXIS_HEIGHT

        painter.drawLine(int(plot_x), y_top, int(plot_x + plot_width), y_top)
        painter.drawLine(int(plot_x), y_bottom, int(plot_x + plot_width), y_bottom)

        # Draw 6 major ticks
        num_ticks = 6
        font = QFont()
        font.setPointSize(9)
        painter.setFont(font)

        for i in range(num_ticks + 1):
            fraction = i / num_ticks
            x = plot_x + fraction * plot_width

            # Compute time at this position
            duration = (end_time - start_time).total_seconds()
            offset = fraction * duration
            from datetime import timedelta
            tick_time = start_time + timedelta(seconds=offset)

            # Draw tick
            painter.drawLine(int(x), y_top, int(x), y_top + 5)
            painter.drawLine(int(x), y_bottom - 5, int(x), y_bottom)

            # Draw label
            time_label = tick_time.strftime("%H:%M")
            painter.drawText(int(x) - 20, y_top - 5, time_label)
            painter.drawText(int(x) - 20, y_bottom + 15, time_label)

    def _draw_idle_gaps(self, painter: QPainter, plot_x: float, plot_width: float,
                        start_time: datetime, end_time: datetime) -> None:
        """Draw idle gaps."""
        if not self._timeline_data or not self._timeline_data.idle_intervals:
            return

        painter.setBrush(IDLE_COLOR)
        painter.setPen(Qt.NoPen)

        for gap_start, gap_end in self._timeline_data.idle_intervals:
            x1 = self._time_to_x(gap_start, plot_x, plot_width, start_time, end_time)
            x2 = self._time_to_x(gap_end, plot_x, plot_width, start_time, end_time)

            rect = QRectF(x1, TIME_AXIS_HEIGHT, x2 - x1, self.height() - 2 * TIME_AXIS_HEIGHT)
            painter.drawRect(rect)

    def _draw_targets(self, painter: QPainter, plot_x: float, plot_width: float,
                      start_time: datetime, end_time: datetime) -> None:
        """Draw target exposure bars and labels."""
        if not self._timeline_data:
            return

        y = TIME_AXIS_HEIGHT

        for idx, target in enumerate(self._timeline_data.targets):
            is_selected = target.obs_id == self._selected_obs_id
            row_height = ROW_HEIGHT + (SELECTED_ROW_EXTRA if is_selected else 0)

            # Draw row background if selected
            if is_selected:
                painter.fillRect(0, int(y), self.width(), int(row_height), SELECTED_BG_COLOR)

            # Draw target label (left margin)
            painter.setPen(Qt.black)
            font = QFont()
            font.setPointSize(9)
            if is_selected:
                font.setBold(True)
            painter.setFont(font)

            label = f"{target.obs_order}. {target.name}"
            painter.drawText(5, int(y + 15), label[:25])  # Truncate long names

            # Draw exposure bar
            if target.start and target.end:
                x1 = self._time_to_x(target.start, plot_x, plot_width, start_time, end_time)
                x2 = self._time_to_x(target.end, plot_x, plot_width, start_time, end_time)

                # Choose color
                color = TARGET_COLORS[idx % len(TARGET_COLORS)]
                if is_selected:
                    color.setAlpha(200)
                else:
                    color.setAlpha(150)

                painter.setBrush(color)
                painter.setPen(QPen(color.darker(120), 1))

                # Draw rounded rectangle
                bar_height = 10
                bar_y = y + (row_height - bar_height) / 2
                rect = QRectF(x1, bar_y, x2 - x1, bar_height)
                painter.drawRoundedRect(rect, 3, 3)

            # Draw airmass curve if selected
            if is_selected and target.airmass and self._timeline_data.times_utc:
                self._draw_airmass_curve(painter, target, plot_x, plot_width,
                                        start_time, end_time, y, row_height)

            # Draw row separator
            painter.setPen(QPen(GRID_COLOR, 1, Qt.DotLine))
            painter.drawLine(0, int(y + row_height), self.width(), int(y + row_height))

            y += row_height

    def _draw_airmass_curve(self, painter: QPainter, target: TimelineTarget,
                           plot_x: float, plot_width: float,
                           start_time: datetime, end_time: datetime,
                           row_y: float, row_height: float) -> None:
        """Draw airmass curve for selected target."""
        if not self._timeline_data or not target.airmass or not self._timeline_data.times_utc:
            return

        # Airmass range
        airmass_min = 1.0
        airmass_max = self._timeline_data.airmass_limit or 4.0

        # Curve area (bottom part of expanded row)
        curve_height = SELECTED_ROW_EXTRA - 10
        curve_y = row_y + ROW_HEIGHT + 5

        # Build path
        path = QPainterPath()
        first_point = True

        for i, (time_str, airmass_val) in enumerate(zip(self._timeline_data.times_utc, target.airmass)):
            if airmass_val is None or airmass_val < airmass_min or airmass_val > airmass_max:
                continue

            x = self._time_to_x(time_str, plot_x, plot_width, start_time, end_time)

            # Scale airmass to curve height (inverted: lower airmass = higher on plot)
            airmass_fraction = (airmass_val - airmass_min) / (airmass_max - airmass_min)
            y = curve_y + curve_height * airmass_fraction

            if first_point:
                path.moveTo(x, y)
                first_point = False
            else:
                path.lineTo(x, y)

        # Draw path
        painter.setPen(QPen(Qt.blue, 2, Qt.DashLine))
        painter.setBrush(Qt.NoBrush)
        painter.drawPath(path)

        # Draw airmass labels (up to 10)
        painter.setPen(QColor(100, 100, 255, 180))
        font = QFont()
        font.setPointSize(8)
        painter.setFont(font)

        label_interval = max(1, len(target.airmass) // 10)
        for i in range(0, len(target.airmass), label_interval):
            if i >= len(self._timeline_data.times_utc):
                break

            time_str = self._timeline_data.times_utc[i]
            airmass_val = target.airmass[i]

            if airmass_val is None or airmass_val < airmass_min or airmass_val > airmass_max:
                continue

            x = self._time_to_x(time_str, plot_x, plot_width, start_time, end_time)
            airmass_fraction = (airmass_val - airmass_min) / (airmass_max - airmass_min)
            y = curve_y + curve_height * airmass_fraction

            label = f"{airmass_val:.2f}"
            painter.drawText(int(x) - 15, int(y) - 5, label)

    def _draw_flags(self, painter: QPainter) -> None:
        """Draw OTM flags in right margin."""
        if not self._timeline_data:
            return

        y = TIME_AXIS_HEIGHT

        for target in self._timeline_data.targets:
            is_selected = target.obs_id == self._selected_obs_id
            row_height = ROW_HEIGHT + (SELECTED_ROW_EXTRA if is_selected else 0)

            if target.flag:
                # Get flag color based on severity
                color = QColor(otm_flag_color(target.severity))
                painter.setPen(color)

                font = QFont()
                font.setPointSize(9)
                if is_selected:
                    font.setBold(True)
                painter.setFont(font)

                flag_x = self.width() - RIGHT_MARGIN + 10
                painter.drawText(int(flag_x), int(y + 15), target.flag)

            y += row_height

    def mousePressEvent(self, event: QMouseEvent) -> None:
        """Handle mouse click."""
        if not self._timeline_data:
            return

        y = TIME_AXIS_HEIGHT

        for target in self._timeline_data.targets:
            is_selected = target.obs_id == self._selected_obs_id
            row_height = ROW_HEIGHT + (SELECTED_ROW_EXTRA if is_selected else 0)

            if y <= event.y() < y + row_height:
                # Clicked on this row
                self.target_selected.emit(target.obs_id)
                return

            y += row_height

    def mouseMoveEvent(self, event: QMouseEvent) -> None:
        """Handle mouse move for hover effects."""
        if not self._timeline_data:
            return

        y = TIME_AXIS_HEIGHT
        hover_row = -1

        for idx, target in enumerate(self._timeline_data.targets):
            is_selected = target.obs_id == self._selected_obs_id
            row_height = ROW_HEIGHT + (SELECTED_ROW_EXTRA if is_selected else 0)

            if y <= event.y() < y + row_height:
                hover_row = idx

                # Show tooltip
                tooltip = f"{target.name}\n"
                if target.start and target.end:
                    tooltip += f"Start: {target.start}\nEnd: {target.end}\n"
                if target.flag:
                    tooltip += f"Flag: {target.flag}"

                QToolTip.showText(event.globalPos(), tooltip)
                break

            y += row_height

        if hover_row != self._hover_row:
            self._hover_row = hover_row
            self.update()
