# NGPS DB GUI (Qt/C++)

This tool provides a Qt Widgets GUI for managing NGPS target sets and targets.

## Features
- Table views populated directly from the database
- Add dialog for new rows; inline edit by double-click
- Manual refresh button to reload from the DB
- Search targets by `NAME` (case-insensitive, partial match)
- Active targets can be reordered (drag/drop or right-click)
- Reordering updates `OBS_ORDER` using `OBSERVATION_ID` to avoid duplicates
- Set View tab tracks the currently selected target set
- Sequencer controls: `seq start` and `seq abort`
- Activate a target set via `seq targetset <SET_ID>`

## Build
From this directory:

```bash
cmake -S . -B build
cmake --build build
```

## Run
```bash
./build/ngps_db_gui
```

## Notes
- The GUI loads DB settings from `Config/sequencerd.cfg` (auto-detected).
- Requires MySQL Connector/C++ (X DevAPI) for direct DB access (e.g. `mysql-connector-c++`).
- The sequencer buttons run the `seq` command. Set `SEQUENCERD_CONFIG` or
  `NGPS_ROOT` in your environment if needed.
- To set a nullable field to NULL in the table, clear the cell or type `NULL`.
