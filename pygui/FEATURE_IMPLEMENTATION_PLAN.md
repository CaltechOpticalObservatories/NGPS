# Database GUI Feature Implementation Plan

## Overview
Complete port of C++ database GUI features (tools/ngps_db_gui/main.cpp ~7972 lines) to Python pygui implementation.

## Completed Features ✅

### Phase 1: Critical UX Conveniences
- ✅ State persistence (QSettings) - window geometry, column widths, tab index
- ✅ Scroll position preservation during refresh
- ✅ Data normalization system (CHANNEL, EXPTIME, SLITWIDTH validation)
- ✅ Transaction support in DbClient
- ✅ Status bar feedback
- ✅ Integration into pygui as "Target Sets" tab

## Implementation Phases

### Phase 2: Context Menu & Basic Table Operations (Week 1-2)
**Priority: HIGH** - Essential for target management workflow

#### 2.1 Context Menu Framework
- [ ] Right-click detection on table rows
- [ ] QMenu creation with dynamic actions based on row state
- [ ] Context menu positioning (at cursor)

#### 2.2 Basic Operations
- [ ] **Move Up/Down**: Reorder targets by swapping OBS_ORDER
- [ ] **Move to Top/Bottom**: Set OBS_ORDER to min-1 or max+1
- [ ] **Delete**: Remove row from database with confirmation dialog
- [ ] **Duplicate**: Copy row with new OBSERVATION_ID and incremented OBS_ORDER
- [ ] **Refresh View**: Reload table after each operation

#### 2.3 Enhanced Edit Dialog
- [ ] `RecordEditorDialog` class - full-featured edit form
- [ ] NULL checkbox support for nullable columns
- [ ] Field validation (same as inline normalization)
- [ ] Apply/Cancel buttons with transaction support

**Files to Modify**:
- `pygui/database_tab.py` - Add context menu handler
- New file: `pygui/record_editor_dialog.py`

---

### Phase 3: Coordinate System & Grouping (Week 3-4)
**Priority: HIGH** - Critical for multi-position science observations

#### 3.1 Gnomonic Projection
- [ ] `compute_science_coord_projected()` - RA/DEC with offset to projected coords
- [ ] Angular separation calculation (Haversine formula)
- [ ] Coordinate key generation (1 arcsec tolerance)

#### 3.2 Automatic Grouping Algorithm
- [ ] Science center identification (targets with science exposures)
- [ ] Group assignment by proximity (≤1 arcsec)
- [ ] Calibration target handling (per-target groups)
- [ ] Manual ungroup tracking (persistent set)

#### 3.3 Group Visualization
- [ ] Group header rows (first science target in group)
- [ ] Expand/collapse icons (▶/▼)
- [ ] Member row indentation (2 spaces)
- [ ] Hide/show members based on expansion state
- [ ] Persistent expansion state (QSettings)

#### 3.4 Group Operations
- [ ] Context menu: "Ungroup" - remove from auto-grouping
- [ ] Context menu: "Regroup" - restore to auto-grouping
- [ ] "Use For Sequencer" submenu - selective activation

**Files to Modify**:
- `pygui/database_tab.py` - Add grouping logic
- New file: `pygui/coordinate_utils.py` - Projection math

**Math Reference**:
```python
# Gnomonic (tangent plane) projection
ra0, dec0 = base_coords  # radians
xi = offset_ra_arcsec / 3600 * deg2rad
eta = offset_dec_arcsec / 3600 * deg2rad
denom = cos(dec0) - eta * sin(dec0)
ra1 = ra0 + atan2(xi, denom)
dec1 = atan2(sin(dec0) + eta * cos(dec0), sqrt(denom**2 + xi**2))
```

---

### Phase 4: OTM Integration (Week 5-6)
**Priority: HIGH** - Core scheduling functionality

#### 4.1 OTM Settings Dialog
- [ ] `OtmSettingsDialog` class with fields:
  - Seeing FWHM (arcsec)
  - Seeing pivot wavelength (nm)
  - Airmass max
  - Use sky simulation checkbox
  - Python executable path
- [ ] Settings persistence (QSettings)

#### 4.2 OTM Input Generation
- [ ] `generate_otm_input_csv()` - Create OTM input file
  - Header: OBSERVATION_ID, name, RA, DECL, slitangle, slitwidth, [NEXP], exptime, notbefore, pointmode, ccdmode, airmass_max, binspat, binspect, channel, wrange_low, wrange_high, magnitude, magfilter, magsystem, [srcmodel]
  - Use projected coordinates for science with offsets
  - Include group headers as science targets
  - Repeat rows for NEXP > 1
- [ ] Temporary file management

#### 4.3 OTM Subprocess Execution
- [ ] `run_otm_scheduler()` - Execute Python OTM script via subprocess
  - Command: `python3 Python/OTM/otm_schedule.py <input.csv> <start_utc> -seeing <fwhm> <pivot> -airmass_max <max> -out <output.csv> [-noskysim]`
  - Working directory: NGPS root
  - Progress dialog with cancel support
  - Stderr/stdout capture for errors

#### 4.4 Timeline JSON Generation
- [ ] `generate_timeline_json()` - Execute timeline script
  - Command: `python3 Python/OTM/otm_timeline.py --input <input.csv> --output <output.csv> --json <timeline.json> --start <start_utc>`
  - Parse JSON timeline format

#### 4.5 OTM Results Import
- [ ] Parse OTM output CSV columns: otmstart, otmend, otmslewgo, otmflag, otmwait, otmdead, otmslew
- [ ] Update database with batch SQL:
  - OTMstart, OTMend, OTMslewgo (timestamps)
  - OTMflag, OTMwait, OTMdead, OTMslew (strings/floats)
  - Recalculate OBS_ORDER based on schedule
- [ ] Refresh table view

#### 4.6 OTM Flag Display
- [ ] Flag severity classification:
  - Severity 2 (error): Contains "DAY-0-1", "DAY-1", or "1" suffix → Red (#DC4646)
  - Severity 1 (warning): Non-empty flag → Orange (#FFA500)
  - Severity 0: Empty → Normal text
- [ ] Flag column in table with color coding
- [ ] Flag tooltips with descriptions

**Files to Modify**:
- `pygui/database_tab.py` - Add OTM menu actions
- New file: `pygui/otm_integration.py` - OTM logic
- New file: `pygui/otm_settings_dialog.py` - Settings UI

**OTM Scripts Required** (verify existence):
- `Python/OTM/otm_schedule.py` - Main scheduler
- `Python/OTM/otm_timeline.py` - Timeline generator

---

### Phase 5: Timeline Visualization (Week 7-9)
**Priority: MEDIUM** - Nice-to-have for visualization

#### 5.1 Timeline Data Model
- [ ] `TimelineData` dataclass:
  - times_utc: List[datetime]
  - twilight times (evening/morning 12°/16°)
  - targets: List[TimelineTarget]
  - idle_intervals: List[Tuple[datetime, datetime]]
- [ ] `TimelineTarget` dataclass:
  - obs_id, name, start, end, slew_go
  - airmass: List[float]
  - flag, severity
  - observed: bool

#### 5.2 TimelineCanvas Widget
- [ ] Custom QWidget with paintEvent override
- [ ] Layout: left margin (200px) + plot area + right margin (172px)
- [ ] Row height: 26px base + 70px for selected (airmass expansion)

#### 5.3 Rendering Components
- [ ] Time axis (6 major ticks, HH:mm format)
- [ ] Twilight markers (dashed gray lines)
- [ ] Exposure bars (rounded rectangles, color-coded)
- [ ] Airmass curves (dashed paths, clipped to [1.0, 4.0])
- [ ] Airmass labels (up to 10 per curve)
- [ ] Idle gap rectangles (semi-transparent red)
- [ ] Row separators (dashed gray)
- [ ] Flag text (right margin, color-coded by severity)

#### 5.4 Interactive Features
- [ ] Mouse click: Select target, emit `targetSelected` signal
- [ ] Right-click: Context menu
- [ ] Drag & drop: Reorder targets
- [ ] Hover: Update cursor and drag preview
- [ ] Tooltip: Show details on hover

#### 5.5 Timeline Tab Integration
- [ ] Add "Timeline" sub-tab to Target Sets tab
- [ ] Load timeline JSON after OTM run
- [ ] Sync selection between table and timeline
- [ ] Update timeline when table data changes

**Files to Create**:
- `pygui/timeline_canvas.py` - Custom painting widget
- `pygui/timeline_data.py` - Data models

---

### Phase 6: Drag & Drop Reordering (Week 10)
**Priority: MEDIUM** - User convenience feature

#### 6.1 Table Drag & Drop
- [ ] Enable drag mode on table widget
- [ ] Detect drag start (press + move threshold)
- [ ] Visual feedback: Closed hand cursor
- [ ] Drop indicator: Blue line between rows
- [ ] Update OBS_ORDER on drop

#### 6.2 Timeline Drag & Drop
- [ ] Drag timeline bars to reorder
- [ ] Snap to valid time slots
- [ ] Update database on drop
- [ ] Refresh both views

**Files to Modify**:
- `pygui/database_tab.py` - Table drag/drop handlers
- `pygui/timeline_canvas.py` - Timeline drag/drop

---

### Phase 7: ETC Integration Enhancements (Week 11)
**Priority: LOW** - Already exists, may need polish

#### 7.1 Review Existing ETC
- [ ] Audit `pygui/etc_popup.py` implementation
- [ ] Verify command format matches C++ version
- [ ] Test all input combinations

#### 7.2 ETC Enhancements
- [ ] Pre-fill fields from selected target row
- [ ] Apply ETC results to multiple selected rows
- [ ] Integrate with OTM workflow (calculate before OTM run)

**Files to Modify**:
- `pygui/etc_popup.py` - Enhancements

---

### Phase 8: Bulk Operations & Advanced Features (Week 12)
**Priority: LOW** - Power user features

#### 8.1 Bulk Column Edit
- [ ] "Set Column Value" dialog
- [ ] Apply to all rows or filtered selection
- [ ] Undo support

#### 8.2 Column Visibility Management
- [ ] Show/hide columns menu
- [ ] Persistent column visibility (QSettings)

#### 8.3 Advanced Filtering
- [ ] Filter by OTM flag severity
- [ ] Filter by observation state
- [ ] Filter by group

#### 8.4 CSV Import/Export
- [ ] Import targets from CSV with validation
- [ ] Export targets to CSV (OTM format)
- [ ] Export timeline to CSV

**Files to Modify**:
- `pygui/database_tab.py` - Add bulk operations
- `pygui/logic_service.py` - CSV handlers

---

## Testing Strategy

### Unit Tests
- [ ] Coordinate projection math (gnomonic)
- [ ] Angular separation calculation
- [ ] Grouping algorithm edge cases
- [ ] OTM CSV generation format
- [ ] Timeline JSON parsing

### Integration Tests
- [ ] OTM subprocess execution
- [ ] Database batch updates
- [ ] Context menu operations
- [ ] Drag & drop reordering

### Manual Testing Checklist
- [ ] Create/edit/delete targets
- [ ] Run OTM scheduler
- [ ] View timeline visualization
- [ ] Group/ungroup targets
- [ ] Reorder via drag/drop
- [ ] Apply ETC calculations

---

## Dependencies

### Python Packages (verify installed)
- `mysqlx-connector-python` - MySQL X DevAPI
- `PyQt5` - GUI framework
- `numpy` - Coordinate calculations (optional, can use math module)

### NGPS Python Scripts (verify existence)
- `Python/OTM/otm_schedule.py` - OTM scheduler
- `Python/OTM/otm_timeline.py` - Timeline generator
- `Python/ETC/ETC_main.py` - Exposure time calculator

### Configuration
- Database schema: Verify all OTM columns exist (OTMstart, OTMend, etc.)
- NGPS_ROOT environment variable for script paths

---

## Estimated Timeline

| Phase | Duration | Effort | Priority |
|-------|----------|--------|----------|
| Phase 2: Context Menu | 1-2 weeks | Medium | HIGH |
| Phase 3: Grouping | 2 weeks | High | HIGH |
| Phase 4: OTM Integration | 2 weeks | High | HIGH |
| Phase 5: Timeline Viz | 3 weeks | High | MEDIUM |
| Phase 6: Drag & Drop | 1 week | Medium | MEDIUM |
| Phase 7: ETC Polish | 1 week | Low | LOW |
| Phase 8: Bulk Ops | 1 week | Medium | LOW |
| **Total** | **10-12 weeks** | | |

---

## Implementation Order (Prioritized)

1. **Week 1-2**: Phase 2 - Context menu (essential for daily use)
2. **Week 3-4**: Phase 3 - Grouping (critical for science operations)
3. **Week 5-6**: Phase 4 - OTM integration (core scheduler functionality)
4. **Week 7-9**: Phase 5 - Timeline visualization (nice-to-have)
5. **Week 10**: Phase 6 - Drag & drop (convenience)
6. **Week 11**: Phase 7 - ETC polish (minor enhancements)
7. **Week 12**: Phase 8 - Bulk operations (power user features)

---

## Notes

- Each phase should include:
  - Implementation
  - Testing
  - Documentation update
  - Git commit with descriptive message

- Maintain backward compatibility with existing pygui functionality
- Follow existing code style and patterns
- Add comprehensive error handling
- Log all subprocess calls and database operations

---

## Success Criteria

The implementation is complete when:
1. ✅ All context menu operations work correctly
2. ✅ Grouping algorithm matches C++ behavior
3. ✅ OTM scheduler runs and imports results successfully
4. ✅ Timeline visualization displays and is interactive
5. ✅ Drag & drop reordering updates database correctly
6. ✅ No feature regressions from C++ version
7. ✅ All tests pass
8. ✅ Documentation is complete

---

*Last Updated: 2026-02-07*
*Target Completion: April 2026*
