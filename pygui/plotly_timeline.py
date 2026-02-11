"""
Plotly-based Gantt-style timeline matching the C++ TimelineCanvas.

Generates a self-contained HTML file with:
- Plotly figure: exposure bars, airmass curves, twilight lines, idle gaps, flags
- Embedded qwebchannel.js for QWebChannel communication
- JS event handlers for click, double-click, right-click, drag-reorder
- selectTargetInTimeline() for Python→JS sync
- Row expansion on selection with airmass numerical labels

Data flow:
    TimelineData → generate_timeline_html() → HTML string → write to temp file
    → QWebEngineView.load()

Plot building is done in JavaScript so it can dynamically rebuild on
selection changes (expanding the selected row and showing airmass labels).
Python serialises raw data; JS constructs all Plotly traces/shapes/annotations.
"""

import colorsys
import hashlib
import json
from datetime import datetime, timedelta, timezone
from typing import List, Optional

try:
    import pytz
    _PALOMAR_TZ = pytz.timezone("US/Pacific")
except ImportError:
    _PALOMAR_TZ = None

try:
    from otm_integration import TimelineData, TimelineTarget, otm_flag_color
except ImportError:
    TimelineData = None
    TimelineTarget = None


def _target_color_hex(obs_id: str, index: int) -> str:
    """Compute HSV-based color from obs_id only (stable across reorders).

    Uses the full MD5 hash to spread hue evenly, ignoring index so that
    a target keeps its color when its position in the list changes.
    """
    h_hash = int(hashlib.md5(obs_id.encode()).hexdigest()[:8], 16)
    h = h_hash % 360
    s = 130.0 / 255.0
    v = 230.0 / 255.0
    r, g, b = colorsys.hsv_to_rgb(h / 360.0, s, v)
    return f"rgb({int(r*255)},{int(g*255)},{int(b*255)})"


def _parse_iso(ts: str) -> Optional[datetime]:
    """Parse ISO timestamp string to datetime."""
    if not ts:
        return None
    try:
        return datetime.fromisoformat(ts.replace('Z', '+00:00'))
    except (ValueError, AttributeError):
        return None


def _flag_severity_color(severity: int) -> str:
    """Get flag annotation color from severity level."""
    if severity == 2:
        return "#FF6666"   # Bright red (visible on dark bg)
    elif severity == 1:
        return "#FFB84D"   # Bright orange (visible on dark bg)
    return "#999999"       # Grey (visible on dark bg)


def _utc_to_local(iso_str: str) -> str:
    """Convert a UTC ISO timestamp string to Palomar local time ISO string.

    Returns the original string unchanged if conversion fails or pytz is
    unavailable.
    """
    if not iso_str or _PALOMAR_TZ is None:
        return iso_str
    try:
        dt = datetime.fromisoformat(iso_str.replace('Z', '+00:00'))
        if dt.tzinfo is None:
            dt = dt.replace(tzinfo=timezone.utc)
        return dt.astimezone(_PALOMAR_TZ).strftime("%Y-%m-%dT%H:%M:%S")
    except Exception:
        return iso_str


def generate_timeline_html(timeline_data, airmass_limit: float = 4.0) -> str:
    """Generate a complete HTML page with Plotly Gantt timeline.

    Args:
        timeline_data: TimelineData from parse_timeline_json()
        airmass_limit: Maximum airmass for y-axis scaling within rows

    Returns:
        Complete HTML string ready to write to a file.
    """
    if not timeline_data or not timeline_data.targets:
        return ("<html><body style='background:#2b2b2b;color:#c0c0c0'>"
                "<p>No timeline data available.</p></body></html>")

    targets = timeline_data.targets
    airmass_max = airmass_limit if airmass_limit > 1.0 else (timeline_data.airmass_limit or 4.0)

    # ── Prepare data for JS ──

    target_js = []
    for i, t in enumerate(targets):
        am_start = ""
        am_end = ""
        try:
            if t.airmass:
                valid = [a for a in t.airmass if a is not None and a > 0]
                if valid:
                    am_start = f"{valid[0]:.2f}"
                    am_end = f"{valid[-1]:.2f}"
        except (TypeError, IndexError):
            pass

        local_start = _utc_to_local(t.start) if t.start else ""
        local_end = _utc_to_local(t.end) if t.end else ""
        hover = (
            f"<b>{t.name}</b><br>"
            f"Start: {local_start}<br>"
            f"End: {local_end}<br>"
            f"Airmass: {am_start} \u2192 {am_end}<br>"
            f"Flag: {t.flag if t.flag else 'OK'}"
        )
        target_js.append({
            "obsId": t.obs_id,
            "name": t.name,
            "obsOrder": t.obs_order,
            "start": local_start,
            "end": local_end,
            "airmass": t.airmass or [],
            "flag": t.flag or "",
            "severity": t.severity,
            "color": _target_color_hex(t.obs_id, i),
            "flagColor": _flag_severity_color(t.severity),
            "hover": hover,
        })

    # Compute time range
    all_times = []
    for t in targets:
        dt = _parse_iso(t.start)
        if dt:
            all_times.append(dt)
        dt = _parse_iso(t.end)
        if dt:
            all_times.append(dt)
    for tw in [timeline_data.twilight_evening_16, timeline_data.twilight_evening_12,
               timeline_data.twilight_morning_12, timeline_data.twilight_morning_16]:
        dt = _parse_iso(tw)
        if dt:
            all_times.append(dt)

    if not all_times:
        return ("<html><body style='background:#2b2b2b;color:#c0c0c0'>"
                "<p>No valid time data.</p></body></html>")

    time_min = min(all_times)
    time_max = max(all_times)
    x_pad = timedelta(minutes=10)

    # Twilight data
    twilight_js = []
    for tw_time, tw_label, tw_color in [
        (timeline_data.twilight_evening_16, "-16\u00b0", "#7799bb"),
        (timeline_data.twilight_evening_12, "-12\u00b0", "#6688aa"),
        (timeline_data.twilight_morning_12, "-12\u00b0", "#6688aa"),
        (timeline_data.twilight_morning_16, "-16\u00b0", "#7799bb"),
    ]:
        if tw_time:
            twilight_js.append({"time": _utc_to_local(tw_time), "label": tw_label, "color": tw_color})

    # Idle intervals
    idle_js = []
    for gap_start, gap_end in timeline_data.idle_intervals:
        if gap_start and gap_end:
            idle_js.append({"start": _utc_to_local(gap_start), "end": _utc_to_local(gap_end)})

    # Convert airmass time grid to local
    times_local = [_utc_to_local(ts) for ts in (timeline_data.times_utc or [])]

    # Convert range boundaries to local
    range_min_local = _utc_to_local((time_min - x_pad).isoformat())
    range_max_local = _utc_to_local((time_max + x_pad).isoformat())
    time_min_local = _utc_to_local(time_min.isoformat())
    time_max_local = _utc_to_local(time_max.isoformat())

    td = {
        "targets": target_js,
        "timesUtc": times_local,
        "airmassMax": airmass_max,
        "twilights": twilight_js,
        "idleIntervals": idle_js,
        "xRangeMin": range_min_local,
        "xRangeMax": range_max_local,
        "timeMin": time_min_local,
        "timeMax": time_max_local,
    }

    td_str = json.dumps(td)

    # ── Build HTML with JS-driven rendering ──
    html = f"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<script src="https://cdn.plot.ly/plotly-2.35.2.min.js"></script>
<script src="qrc:///qtwebchannel/qwebchannel.js"></script>
<style>
  html, body {{ margin: 0; padding: 0; overflow-y: auto; font-family: sans-serif;
               background: #2b2b2b; color: #c0c0c0; }}
  #timeline {{ width: 100%; }}
  .xtick text {{ transform: translateX(-50px); }}
</style>
</head>
<body>
<div id="timeline"></div>

<script>
// ── Data & state ──
var bridge = null;
var TD = {td_str};
var selectedObsId = null;
var currentPos = [];      // {{center, height, selected}} per target
var dragState = null;

var NORMAL_H  = 1.0;
var EXPAND_H  = 3.5;
var BAR_HALF  = 0.15;
var BAND_FRAC = 0.85;    // fraction of row height used for airmass band

// ── Initialise ──
document.addEventListener("DOMContentLoaded", function() {{
    if (typeof QWebChannel !== 'undefined') {{
        new QWebChannel(qt.webChannelTransport, function(ch) {{
            bridge = ch.objects.bridge;
        }});
    }}
    renderPlot(null);
    setupEvents();
}});

// ── Compute y-positions (dynamic row heights) ──
function computePos(selId) {{
    var pos = [], tv = [], tt = [], y = 0;
    for (var i = 0; i < TD.targets.length; i++) {{
        var t = TD.targets[i];
        var h = (t.obsId === selId) ? EXPAND_H : NORMAL_H;
        var c = y + h / 2;
        pos.push({{center: c, height: h, selected: t.obsId === selId}});
        tv.push(c);
        tt.push(t.obsOrder + ". " + t.name);
        y += h;
    }}
    return {{pos: pos, tickVals: tv, tickTexts: tt, totalH: y}};
}}

// ── Build all Plotly data from scratch ──
function buildData(selId) {{
    var yi = computePos(selId);
    currentPos = yi.pos;
    var traces = [], shapes = [], anns = [];
    var n = TD.targets.length;

    // Selection highlight band
    for (var i = 0; i < n; i++) {{
        if (yi.pos[i].selected) {{
            shapes.push({{
                type:'rect', x0:TD.xRangeMin, x1:TD.xRangeMax,
                y0: yi.pos[i].center - yi.pos[i].height/2,
                y1: yi.pos[i].center + yi.pos[i].height/2,
                fillcolor:'rgba(80,140,200,0.15)', line:{{width:0}}, layer:'below'
            }});
        }}
    }}

    // Exposure bars
    for (var i = 0; i < n; i++) {{
        var t = TD.targets[i];
        if (!t.start || !t.end) continue;
        var yc = yi.pos[i].center;
        var sel = yi.pos[i].selected;
        shapes.push({{
            type:'rect', x0:t.start, x1:t.end,
            y0: yc - BAR_HALF, y1: yc + BAR_HALF,
            fillcolor: t.color, opacity: sel ? 0.95 : 0.85,
            line:{{width:1, color:t.color}}, layer:'above'
        }});
    }}

    // Click-detection scatter (invisible markers at bar centres)
    var cx=[], cy=[], cid=[], ctxt=[], ccol=[];
    for (var i = 0; i < n; i++) {{
        var t = TD.targets[i];
        if (t.start && t.end) {{
            var s = new Date(t.start).getTime();
            var e = new Date(t.end).getTime();
            cx.push(new Date((s+e)/2).toISOString());
        }} else {{ cx.push(null); }}
        cy.push(yi.pos[i].center);
        cid.push(t.obsId);
        ctxt.push(t.hover);
        ccol.push(t.color);
    }}
    traces.push({{
        x:cx, y:cy, customdata:cid, text:ctxt,
        hovertemplate: "%{{text}}<extra></extra>",
        mode:'markers', marker:{{size:20, opacity:0, color:ccol}},
        showlegend:false, name:'targets'
    }});

    // Airmass curves
    if (TD.timesUtc.length > 0) {{
        for (var i = 0; i < n; i++) {{
            var t = TD.targets[i];
            if (!t.airmass || t.airmass.length === 0) continue;
            var yc = yi.pos[i].center;
            var h  = yi.pos[i].height;
            var sel = yi.pos[i].selected;
            var bandH = h * BAND_FRAC;
            var amX=[], amY=[];
            for (var j = 0; j < t.airmass.length && j < TD.timesUtc.length; j++) {{
                var am = t.airmass[j];
                if (am === null || am < 1.0 || am > TD.airmassMax) {{
                    amX.push(null); amY.push(null); continue;
                }}
                amX.push(TD.timesUtc[j]);
                var frac = (am - 1.0) / (TD.airmassMax - 1.0);
                amY.push(yc - bandH/2 + frac * bandH);
            }}
            var curveColor = (t.severity >= 2) ? '#666666' : t.color;
            traces.push({{
                x:amX, y:amY, mode:'lines',
                line:{{dash:'dash', width: sel ? 2.5 : 1.5, color:curveColor}},
                hoverinfo:'skip', showlegend:false, connectgaps:false
            }});

            // Airmass labels (only on expanded/selected row)
            if (sel) {{
                addAirmassLabels(anns, t, yc, bandH);
            }}
        }}
    }}

    // Twilight lines & labels
    for (var k = 0; k < TD.twilights.length; k++) {{
        var tw = TD.twilights[k];
        shapes.push({{
            type:'line', x0:tw.time, x1:tw.time,
            y0:-0.5, y1: yi.totalH - 0.5,
            line:{{dash:'dash', width:1, color:tw.color}}, layer:'below'
        }});
        anns.push({{
            x:tw.time, y:-0.5, yref:'y', text:tw.label,
            showarrow:false, font:{{size:9, color:tw.color}},
            yshift:12, clicktoshow:false
        }});
    }}

    // Target name labels (annotations instead of y-axis ticks for per-target colors)
    for (var i = 0; i < n; i++) {{
        var t = TD.targets[i];
        var nameColor = (t.severity >= 2) ? '#707070' : '#e0e0e0';
        anns.push({{
            x:0, xref:'paper', xanchor:'right',
            y: yi.pos[i].center, yref:'y',
            text: t.obsOrder + '. ' + t.name,
            showarrow:false,
            font:{{size:11, color:nameColor}},
            xshift:-8
        }});
    }}

    // Idle gaps
    for (var k = 0; k < TD.idleIntervals.length; k++) {{
        var g = TD.idleIntervals[k];
        shapes.push({{
            type:'rect', x0:g.start, x1:g.end,
            y0:-0.5, y1: yi.totalH - 0.5,
            fillcolor:'rgba(220,50,50,0.18)', line:{{width:0}}, layer:'below'
        }});
    }}

    // Row separators
    var sepY = 0;
    for (var i = 0; i < n - 1; i++) {{
        sepY += yi.pos[i].height;
        shapes.push({{
            type:'line', x0:TD.timeMin, x1:TD.timeMax,
            y0:sepY, y1:sepY,
            line:{{dash:'dot', width:0.5, color:'rgba(100,100,100,0.7)'}}, layer:'below'
        }});
    }}

    // Flag annotations (right margin)
    for (var i = 0; i < n; i++) {{
        var t = TD.targets[i];
        if (!t.flag) continue;
        anns.push({{
            x:1.0, xref:'paper', xanchor:'left',
            y: yi.pos[i].center, yref:'y',
            text:'  ' + t.flag, showarrow:false,
            font:{{size:10, color:t.flagColor, family:'monospace'}},
            captureevents:true, clicktoshow:false, name:'flag_' + t.obsId
        }});
    }}

    return {{traces:traces, shapes:shapes, anns:anns,
             tickVals:yi.tickVals, tickTexts:yi.tickTexts, totalH:yi.totalH}};
}}

// ── Airmass numerical labels along the curve (selected row only) ──
function addAirmassLabels(anns, target, yCenter, bandH) {{
    var am = target.airmass;
    if (!am || am.length === 0) return;

    // Collect visible indices
    var vis = [];
    for (var j = 0; j < am.length && j < TD.timesUtc.length; j++) {{
        if (am[j] !== null && am[j] >= 1.0 && am[j] <= TD.airmassMax) vis.push(j);
    }}
    if (vis.length === 0) return;

    var labelCount = Math.min(8, vis.length);

    for (var k = 0; k < labelCount; k++) {{
        var idx = vis[Math.round((k * (vis.length - 1)) / Math.max(1, labelCount - 1))];
        var v = am[idx];
        var frac = (v - 1.0) / (TD.airmassMax - 1.0);
        var y = yCenter - bandH/2 + frac * bandH;
        anns.push({{
            x: TD.timesUtc[idx], y: y,
            text: v.toFixed(1), showarrow: false,
            font: {{size: 9, color: 'rgba(210,210,210,0.9)'}},
            yshift: -12,
            xanchor: 'center'
        }});
    }}
}}

// ── Render / update plot via Plotly.newPlot ──
function renderPlot(selId) {{
    var pd = buildData(selId);
    var layout = {{
        xaxis: {{
            type:'date', showgrid:true, gridcolor:'rgba(80,80,80,0.6)',
            side:'top', tickfont:{{color:'#c0c0c0', size:11}},
            linecolor:'#555', range:[TD.xRangeMin, TD.xRangeMax]
        }},
        yaxis: {{
            showticklabels:false,
            showgrid:false, fixedrange:true, title:'',
            range:[pd.totalH - 0.5, -0.5]
        }},
        shapes: pd.shapes,
        annotations: pd.anns,
        margin: {{l:180, r:100, t:28, b:10}},
        hovermode: 'closest',
        hoverlabel: {{bgcolor:'#3a3a3a', bordercolor:'#666', font:{{color:'#e0e0e0'}}}},
        plot_bgcolor: '#2b2b2b',
        paper_bgcolor: '#2b2b2b',
        height: Math.max(400, 40 + pd.totalH * 40),
        dragmode: false
    }};
    var config = {{displayModeBar:false, scrollZoom:false, doubleClick:false}};
    Plotly.newPlot('timeline', pd.traces, layout, config);

    // Re-register Plotly event handlers (newPlot purges them)
    var plotDiv = document.getElementById('timeline');
    plotDiv.on('plotly_clickannotation', function(evt) {{
        var ann = evt.annotation;
        if (ann && ann.name && ann.name.startsWith('flag_')) {{
            var obsId = ann.name.substring(5);
            var flagText = (ann.text || '').trim();
            if (bridge) bridge.onFlagClicked(obsId, flagText);
        }}
    }});
}}

// ── Find obs_id at a given data-Y coordinate ──
function getObsIdAtY(dataY) {{
    for (var i = 0; i < currentPos.length; i++) {{
        var p = currentPos[i];
        if (dataY >= p.center - p.height/2 && dataY <= p.center + p.height/2)
            return TD.targets[i].obsId;
    }}
    // Fallback: nearest row
    var best = Infinity, id = null;
    for (var i = 0; i < currentPos.length; i++) {{
        var d = Math.abs(dataY - currentPos[i].center);
        if (d < best) {{ best = d; id = TD.targets[i].obsId; }}
    }}
    return id;
}}

function getObsIdFromEvent(evt) {{
    var plotDiv = document.getElementById('timeline');
    var drag = plotDiv.querySelector('.plotly .nsewdrag');
    if (!drag) return null;
    var bb = drag.getBoundingClientRect();
    var yaxis = plotDiv._fullLayout.yaxis;
    return getObsIdAtY(yaxis.p2d(evt.clientY - bb.top));
}}

// ── Event handlers ──
function setupEvents() {{
    var plotDiv = document.getElementById('timeline');
    var suppressClick = false;  // suppress click after drag-reorder

    // Single click → select & expand, double-click → edit exptime
    // Uses native DOM events so they survive Plotly.newPlot rebuilds
    var lastClickTime = 0, lastClickObs = null;
    plotDiv.addEventListener('click', function(evt) {{
        if (suppressClick) {{ suppressClick = false; return; }}
        var now = Date.now();
        var obsId = getObsIdFromEvent(evt);
        if (!obsId) return;
        if ((now - lastClickTime < 400) && lastClickObs === obsId) {{
            // Double-click
            if (bridge) bridge.onTargetDoubleClicked(obsId);
            lastClickTime = 0; lastClickObs = null;
        }} else {{
            // Single click → select & expand
            lastClickTime = now; lastClickObs = obsId;
            if (obsId !== selectedObsId) {{
                selectedObsId = obsId;
                renderPlot(selectedObsId);
                if (bridge) bridge.onTargetClicked(obsId);
            }}
        }}
    }});

    // Right-click context menu
    plotDiv.addEventListener('contextmenu', function(evt) {{
        evt.preventDefault();
        var obsId = getObsIdFromEvent(evt);
        if (obsId && bridge) bridge.onContextMenu(obsId, evt.screenX, evt.screenY);
    }});

    // Drag-to-reorder (mousedown anywhere on a target row)
    var dragOverlay = null;
    var DRAG_THRESHOLD = 6;  // pixels before drag activates
    plotDiv.addEventListener('mousedown', function(evt) {{
        if (evt.button !== 0) return;
        var obsId = getObsIdFromEvent(evt);
        if (!obsId) return;
        dragState = {{fromObsId: obsId, startY: evt.clientY, active: false}};
    }});

    document.addEventListener('mousemove', function(evt) {{
        if (!dragState) return;
        // Activate drag only after moving past threshold
        if (!dragState.active) {{
            if (Math.abs(evt.clientY - dragState.startY) < DRAG_THRESHOLD) return;
            dragState.active = true;
        }}
        var plotDiv = document.getElementById('timeline');
        var drag = plotDiv.querySelector('.plotly .nsewdrag');
        if (!drag) return;
        var bb = drag.getBoundingClientRect();
        var yaxis = plotDiv._fullLayout.yaxis;
        var dataY = yaxis.p2d(evt.clientY - bb.top);
        if (!dragOverlay) {{
            dragOverlay = document.createElement('div');
            dragOverlay.style.cssText = 'position:fixed;height:2px;background:#5599ff;z-index:9999;pointer-events:none;';
            document.body.appendChild(dragOverlay);
        }}
        var obsId = getObsIdAtY(dataY);
        if (obsId) {{
            for (var i = 0; i < TD.targets.length; i++) {{
                if (TD.targets[i].obsId === obsId) {{
                    var sepYdata = currentPos[i].center + currentPos[i].height / 2;
                    var sepYpx = bb.top + yaxis.d2p(sepYdata);
                    dragOverlay.style.top = sepYpx + 'px';
                    dragOverlay.style.left = bb.left + 'px';
                    dragOverlay.style.width = bb.width + 'px';
                    break;
                }}
            }}
        }}
    }});

    document.addEventListener('mouseup', function(evt) {{
        if (!dragState) return;
        if (dragState.active) {{
            suppressClick = true;  // prevent click handler from rebuilding with stale data
            var obsId = getObsIdFromEvent(evt);
            if (obsId && obsId !== dragState.fromObsId && bridge) {{
                bridge.onReorder(dragState.fromObsId, obsId);
            }}
        }}
        if (dragOverlay) {{ dragOverlay.remove(); dragOverlay = null; }}
        dragState = null;
    }});
}}

// ── Python→JS: select/highlight target from table ──
function selectTargetInTimeline(obsId) {{
    selectedObsId = obsId;
    renderPlot(obsId);
    // Scroll into view if needed
    for (var i = 0; i < TD.targets.length; i++) {{
        if (TD.targets[i].obsId === obsId) {{
            var plotDiv = document.getElementById('timeline');
            var yaxis = plotDiv._fullLayout.yaxis;
            var drag = plotDiv.querySelector('.plotly .nsewdrag');
            if (drag) {{
                var bb = drag.getBoundingClientRect();
                var rowPx = yaxis.d2p(currentPos[i].center);
                var absY = bb.top + rowPx;
                if (absY < 0 || absY > window.innerHeight) {{
                    window.scrollTo(0, Math.max(0, absY - window.innerHeight / 2));
                }}
            }}
            break;
        }}
    }}
}}
</script>
</body>
</html>"""

    return html
