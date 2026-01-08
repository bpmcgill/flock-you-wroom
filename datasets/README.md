# Datasets Folder

This folder contains legacy detection data files that can be converted to the new database format.

## Current Files

- `Flock-_______20240530_124303.csv` - WiFi scan results
- `FS+Ext+Battery_20240530_105846.csv` - Extended scan data
- `maximum_dots.csv` - Device list
- `Penguin-___________20240530_111436.csv` - Named scan session
- `Pigvision.csv` - Device database
- `raven_configurations.json` - Raven device configuration database

## Converting to New Format

### Using PowerShell Script

```powershell
# From the repo root directory
.\convert-datasets.ps1
```

This will:
1. Scan all CSV and JSON files in this folder
2. Parse and aggregate detection data
3. Create `/detections.db` (main database)
4. Create `/device_index.idx` (fast lookup)
5. Generate exports in `/exports/` folder

### Output Files

After conversion:
```
datasets/
├── detections.db              # Main database (load onto SD card)
├── device_index.idx           # Index file (load onto SD card)
├── locations.db               # GPS locations (if available)
└── exports/
    ├── summary.csv            # Human-readable summary
    └── detections.geojson     # Map visualization
```

### Loading onto ESP32

1. Run conversion script
2. Copy these files to SD card root:
   - `detections.db`
   - `device_index.idx`
   - `locations.db` (if exists)
3. Insert SD card into ESP32
4. Power on - device loads known devices into memory

## File Formats Supported

The conversion script automatically detects and handles:

### CSV with Headers
```csv
MAC,SSID,RSSI,Channel
AA:BB:CC:DD:EE:FF,FlockCamera,-65,6
```

### CSV without Headers
```csv
AA:BB:CC:DD:EE:FF,WiFi,-65,40.7128,-74.0060
```

### JSON Lines
```json
{"mac":"AA:BB:CC:DD:EE:FF","type":"BLE","rssi":-65}
```

### JSON Array
```json
[
  {"mac":"AA:BB:CC:DD:EE:FF","name":"Device 1"},
  {"mac":"11:22:33:44:55:66","name":"Device 2"}
]
```

## Merging with Existing Database

To add this data to an already-running detector:

```powershell
# Merge with existing database instead of overwriting
.\convert-datasets.ps1 -Merge
```

This combines:
- Historical data from these files
- Current detections from device's SD card
- Preserves detection counts and locations
