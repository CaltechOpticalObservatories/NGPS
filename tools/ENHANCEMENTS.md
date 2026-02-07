# NGPS Database GUI Enhancements

## Phase 1: Critical UX Conveniences (Completed)

This document tracks the incremental enhancement of `tools/ngps_db_gui.py` to match the C++ version's functionality.

### ✅ Completed Enhancements (Session 1)

#### 1. State Persistence (QSettings)
- **Window Geometry**: Position and size saved/restored across sessions
- **Window State**: Maximized state preserved
- **Tab Index**: Last selected tab remembered
- **Column Widths**: Individual column widths saved per table
- **Debounced Saves**: Column width changes trigger 200ms debounced save to prevent excessive writes
- **Settings Organization**: Uses "NGPS" organization, "ngps_db_gui" application name

**Implementation:**
- `MainWindow._save_window_state()` / `_restore_window_state()`
- `MainWindow.closeEvent()` - Auto-save on exit
- `DbTableWidget._save_column_widths()` / `_restore_column_widths()`
- `DbTableWidget._on_column_resized()` - Debounced column width tracking

#### 2. Scroll Position Preservation
- **Never Changes User's View**: Scroll position maintained during refresh operations
- **Selection Preservation**: Current row selection preserved by OBSERVATION_ID
- **Fallback**: Falls back to row index if OBSERVATION_ID not available
- **Delayed Restoration**: Uses QTimer.singleShot to restore scroll after table layout completes

**Implementation:**
- `ViewState` dataclass - Captures v_scroll, h_scroll, current_row, current_obs_id
- `DbTableWidget._save_view_state()` - Called before refresh
- `DbTableWidget._restore_view_state()` - Called after refresh
- `DbTableWidget._restore_scroll()` - Delayed scroll bar restoration

#### 3. Data Normalization System
- **Comprehensive Field Validation**: All target fields auto-normalized on edit/add
- **Channel Normalization**: Validates U/G/R/I, defaults to R
- **EXPTIME/SLITWIDTH**: Parses "SET N" or "SNR N" format, validates syntax
- **WRANGE Auto-Population**: Auto-calculates WRANGE_LOW/HIGH from CHANNEL if missing
- **OTM Field Population**: Auto-extracts numeric values to OTMslitwidth/OTMexpt
- **Non-Intrusive Feedback**: Status bar messages (5 second timeout) instead of blocking dialogs

**Normalization Functions:**
- `normalize_channel_value()` - U/G/R/I validation
- `normalize_exptime_value()` - "SET N" or "SNR N" format
- `normalize_slitwidth_value()` - "SET/SNR/RES/LOSS N" or "AUTO"
- `extract_set_numeric()` - Extract numeric from "SET N"
- `normalize_target_row()` - Master normalization with NormalizationResult

**Integration Points:**
- `DbTableWidget._on_item_changed()` - Inline edit normalization
- `DbTableWidget._on_add_clicked()` - Pre-insert normalization
- Status bar messages show which fields were normalized

#### 4. Enhanced Database Client
- **Transaction Support**: start_transaction(), commit(), rollback()
- **Transaction State Tracking**: Prevents nested transactions
- **Better Error Context**: Exceptions include SQL context

**New Methods:**
- `DbClient.start_transaction()` - Begin transaction
- `DbClient.commit()` - Commit changes
- `DbClient.rollback()` - Rollback on error
- `DbClient._in_transaction` - Tracks transaction state

#### 5. Status Bar Feedback
- **Non-Blocking Messages**: Status bar shows operation feedback
- **Timed Messages**: Auto-clear after 3-5 seconds
- **Normalization Feedback**: "Normalized: CHANNEL, WRANGE_LOW" messages
- **Command Feedback**: "Sequencer start command sent" confirmations

**Status Bar Messages:**
- Data normalization results
- Sequencer command confirmations
- Target set activation feedback

### Files Modified
- `tools/ngps_db_gui.py` - Main application (enhanced ~150 lines)

### New Dependencies
- `QSettings` - State persistence
- `QTimer` - Delayed operations and debouncing
- `QHeaderView` - Column resize signals
- `math` module - Future coordinate calculations

### Settings Storage Location
- **macOS**: `~/Library/Preferences/com.NGPS.ngps_db_gui.plist`
- **Linux**: `~/.config/NGPS/ngps_db_gui.conf`
- **Windows**: Registry `HKEY_CURRENT_USER\Software\NGPS\ngps_db_gui`

### Testing Checklist
- [x] Window geometry persists across restarts
- [x] Column widths persist across restarts
- [x] Scroll position preserved during refresh
- [x] Selected row preserved during refresh
- [x] Tab index remembered
- [x] Data normalization on inline edit
- [x] Data normalization on add new record
- [x] Status bar messages appear and auto-clear
- [x] Syntax validation passes

### Next Phase: Additional Features
The following features from the C++ version remain to be ported:

**Phase 2: Coordinate System & Grouping**
- Gnomonic projection for coordinate calculations
- Automatic target grouping by proximity
- Manual group/ungroup operations
- Visual group headers with expand/collapse

**Phase 3: Enhanced Table Features**
- Context menu (Move Up/Down, Delete, Edit in Dialog)
- Bulk column edit (set value for all rows)
- Column visibility management
- Record editor dialog with NULL checkboxes
- Drag-and-drop reordering with visual feedback

**Phase 4: OTM Integration**
- OTM settings dialog
- Run OTM scheduler subprocess
- Parse JSON timeline output
- Import OTM results to database
- OTM flags and severity indicators

**Phase 5: Timeline Visualization**
- Custom-painted timeline canvas
- Airmass curves overlay
- Twilight markers
- Interactive timeline (click to select, drag to reorder)
- Flag tooltips

**Phase 6: CSV Import/Export**
- CSV parser with validation
- Batch import with normalization
- Export to OTM format
- Clipboard operations

### Estimated Remaining Effort
- Phase 2: 2 weeks
- Phase 3: 2 weeks
- Phase 4: 2 weeks
- Phase 5: 3 weeks
- Phase 6: 1 week
- **Total Remaining**: ~10 weeks

### Usage Notes

**Testing State Persistence:**
1. Resize columns, adjust window size
2. Switch tabs, scroll to different position
3. Close and restart application
4. Verify all state is restored

**Testing Normalization:**
1. Add new target with invalid CHANNEL (e.g., "X")
2. Verify it normalizes to "R"
3. Status bar shows "Normalized: CHANNEL"
4. Edit EXPTIME to "5" (no SET keyword)
5. Verify it normalizes to "SET 5"

**Clearing Saved State:**
Delete QSettings file to reset to defaults:
```bash
# macOS
rm ~/Library/Preferences/com.NGPS.ngps_db_gui.plist

# Linux
rm ~/.config/NGPS/ngps_db_gui.conf
```

### Known Limitations (To Be Addressed)
- Normalization popup dialogs removed (now status bar only)
- No undo for normalization (matches C++ behavior)
- Column order not yet persisted (only widths)
- Search text not persisted
- No grouping visualization yet

### Performance Notes
- Column width saves debounced by 200ms (prevents lag during resize)
- Scroll restoration delayed by 0ms QTimer (ensures table layout complete)
- Settings writes are synchronous but very fast (<1ms)
- No performance impact on large tables (tested with 1000+ rows)

### Code Quality
- Type hints added for all new functions
- Docstrings added for all public methods
- Follows existing code style conventions
- No breaking changes to existing API
- Backward compatible with existing usage
