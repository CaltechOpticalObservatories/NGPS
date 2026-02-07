# Database GUI Integration into pygui

## Overview

The enhanced database GUI from `tools/ngps_db_gui.py` has been integrated into the main `pygui/ngps_gui.py` application as a new "Target Sets" tab.

## What Was Added

### New Files

1. **`pygui/database_tab.py`** (~700 lines)
   - `DatabaseTab` widget - Main container for target sets and targets tables
   - `DatabaseTableWidget` - Enhanced table widget with all convenience features
   - `DbClient` - Database client with transaction support
   - Imports normalization functions from `tools/ngps_db_gui.py`

### Modified Files

1. **`pygui/layout_service.py`**
   - Added import for `DatabaseTab`
   - Added "Target Sets" tab to third column
   - Added `initialize_database_tab()` method

2. **`pygui/ngps_gui.py`**
   - Calls `layout_service.initialize_database_tab()` after services are initialized

## Features Integrated

### ✅ State Persistence
- Column widths saved per table
- Scroll position preserved during refresh
- Selection preserved by OBSERVATION_ID
- Settings stored separately from standalone tool (NGPS/ngps_gui_database)

### ✅ Data Normalization
- All target fields auto-normalized on edit
- CHANNEL, EXPTIME, SLITWIDTH validation
- WRANGE auto-population
- OTM field extraction
- Status bar messages for feedback

### ✅ Enhanced UX
- Never changes user's view during refresh
- Debounced column width saves (200ms)
- Delayed scroll restoration
- Transaction support for database operations

### ✅ Two-Table View
- Target Sets table (read-only)
- Targets table (filtered by selected set)
- Automatic filtering when set is selected
- Search functionality on NAME column

## Usage

### Running the Integrated GUI

```bash
cd /Users/cfremling/Projects/NGPS_codebase/NGPS/pygui
python3 ngps_gui.py
```

The new "Target Sets" tab will appear in the third column alongside the Control tab.

### Configuration

The database tab reads configuration from:
1. `$NGPS_CONFIG` environment variable
2. `$NGPS_ROOT/Config/sequencerd.cfg`
3. Auto-detected from file location

Required config keys:
- `DB_HOST`, `DB_PORT`, `DB_USER`, `DB_PASS`
- `DB_SCHEMA`, `DB_ACTIVE` (targets table), `DB_SETS` (sets table)

### State Persistence Location

Settings are stored in:
- **macOS**: `~/Library/Preferences/com.NGPS.ngps_gui_database.plist`
- **Linux**: `~/.config/NGPS/ngps_gui_database.conf`
- **Windows**: Registry `HKEY_CURRENT_USER\Software\NGPS\ngps_gui_database`

## Architecture

### Integration Pattern

```
NgpsGUI (main window)
  └─> LayoutService.create_third_column()
       └─> Creates QTabWidget with tabs:
            - Control Tab
            - Target Sets Tab (new!)
                └─> DatabaseTab widget
                     ├─> DatabaseTableWidget (Sets)
                     └─> DatabaseTableWidget (Targets)
```

### Data Flow

```
User edits cell
  └─> DatabaseTableWidget._on_item_changed()
       ├─> normalize_target_row() [from tools.ngps_db_gui]
       ├─> Update all normalized fields
       └─> Show status message (via parent window status bar)
```

### State Persistence Flow

```
Column resize
  └─> QTimer debounce (200ms)
       └─> DatabaseTableWidget._save_column_widths()
            └─> QSettings.setValue()

Refresh table
  ├─> Save: _save_view_state()
  ├─> Clear and repopulate table
  └─> Restore: _restore_view_state()
       └─> QTimer.singleShot(0) for scroll bars
```

## Testing

### Manual Testing Checklist

- [ ] Tab appears in GUI third column
- [ ] Database connection status shows correctly
- [ ] Target sets table populates
- [ ] Selecting a set filters targets table
- [ ] Scroll position preserved during refresh
- [ ] Column widths persist across restarts
- [ ] Inline editing works
- [ ] Data normalization triggers on edit
- [ ] Status bar shows normalization messages
- [ ] Search filter works on targets table
- [ ] Delete button removes rows

### Testing Data Normalization

1. Edit a target's CHANNEL to "X" → Should normalize to "R"
2. Edit EXPTIME to "5" → Should normalize to "SET 5"
3. Edit SLITWIDTH to "AUTO" → Should keep "AUTO"
4. Watch status bar for "Normalized: ..." messages

### Testing State Persistence

1. Resize several columns
2. Scroll to middle of target list
3. Select a specific row
4. Close entire application
5. Reopen application
6. Verify:
   - Column widths restored
   - Scroll position near where you left off
   - Same row selected (if still exists)

## Dependencies

### Python Modules Required

```
mysqlx          # MySQL X DevAPI connector
PyQt5           # Qt5 Python bindings
```

Install with:
```bash
pip install mysqlx-connector-python PyQt5
```

### Shared Code

The database tab imports these from `tools/ngps_db_gui.py`:
- `ColumnMeta`, `DbConfig`, `NormalizationResult`, `ViewState` (data classes)
- All normalization functions
- Default constants
- Config loading functions

This ensures consistency between standalone and integrated versions.

## Known Limitations

### Current Session
- Add/Edit dialog not yet ported (shows placeholder message)
- Context menu not implemented
- Drag/drop reordering not functional yet
- Sequencer integration (activate set) shows placeholder

### Future Enhancements (Next Sessions)
- Full Add/Edit dialog with NULL checkboxes
- Context menu (Move Up/Down, Delete, Edit in Dialog)
- Drag/drop visual feedback
- Bulk column edit
- OTM integration
- Timeline visualization
- CSV import/export

## Comparison: Standalone vs Integrated

| Feature | Standalone (`tools/ngps_db_gui.py`) | Integrated (`pygui/database_tab.py`) |
|---------|--------------------------------------|--------------------------------------|
| Window geometry persistence | ✅ Yes | ⚠️ Managed by main window |
| Column width persistence | ✅ Yes | ✅ Yes |
| Scroll position preservation | ✅ Yes | ✅ Yes |
| Data normalization | ✅ Yes | ✅ Yes |
| Transaction support | ✅ Yes | ✅ Yes |
| Status bar messages | ✅ Yes | ✅ Yes (uses main window status bar) |
| Settings key | `NGPS/ngps_db_gui` | `NGPS/ngps_gui_database` |
| Tab management | ⚠️ Own window | ✅ Part of main GUI |
| Sequencer controls | ✅ Yes | ⚠️ Placeholder |

## Troubleshooting

### Tab doesn't appear
- Check console for "Database tab initialized successfully"
- Check for import errors in database_tab.py
- Verify mysqlx module is installed

### Database connection fails
- Check `Config/sequencerd.cfg` exists and has correct DB settings
- Verify MySQL X Protocol port (33060) is correct
- Check database credentials

### State not persisting
- Verify QSettings can write to system location
- Check permissions on settings directory
- Try clearing settings file and restarting

### Normalization not working
- Check that `tools/ngps_db_gui.py` exists
- Verify imports succeed (check console output)
- Check parent window has `statusBar()` method

## Next Steps

To continue the integration in future sessions:

1. **Add Full RecordEditorDialog** - Port the comprehensive edit dialog with NULL checkboxes
2. **Implement Context Menu** - Right-click operations (Move Up/Down, Delete, Edit)
3. **Add Drag/Drop Reordering** - Visual feedback for table row reordering
4. **Integrate with Sequencer** - Connect "Activate Target Set" to actual `seq targetset` command
5. **Add Grouping** - Port coordinate-based target grouping
6. **OTM Integration** - Add OTM scheduler execution and result parsing
7. **Timeline Tab** - Add timeline visualization sub-tab

## Code Quality

- Type hints throughout
- Docstrings for all public methods
- Error handling with try/except
- Graceful degradation (works without mysqlx if needed)
- Follows existing pygui code style
- No breaking changes to existing GUI

## Files Modified Summary

```
pygui/database_tab.py           +700 lines (new file)
pygui/layout_service.py         +25 lines
pygui/ngps_gui.py               +8 lines
pygui/DATABASE_INTEGRATION.md   +320 lines (this file)
```

**Total: ~1053 lines added**
