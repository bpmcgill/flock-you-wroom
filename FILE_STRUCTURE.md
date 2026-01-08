# FlockYouWroom File Structure

## Files Embedded on ESP32 (Compiled into firmware)

These files are compiled into the device firmware and cannot be changed without reflashing:

### Configuration (Hardcoded)
```
src/config/
├── pins.h                    # Hardware pin assignments (GPIO mapping)
└── patterns.h                # Detection patterns (MAC prefixes, SSIDs, UUIDs)
```

### Core Application Code
```
src/
├── main.cpp                  # Main program loop
├── CMakeLists.txt           # Build configuration
│
├── detection/               # Detection engines
│   ├── ble_detector.cpp/h
│   ├── wifi_detector.cpp/h
│   ├── raven_detector.cpp/h
│   └── detection_state.cpp/h
│
└── hardware/                # Hardware drivers
    ├── led_controller.cpp/h
    ├── buzzer.cpp/h
    ├── display.cpp/h
    ├── gps_manager.cpp/h
    ├── sd_logger.cpp/h
    └── data_manager.cpp/h    # Database interface (code only)
```

---

## Files Stored on SD Card (Runtime data)

These files are created and updated during operation and persist across reboots:

### Database Files (Persistent Detection History)
```
/detections.db               # Main database (MAC,Type,RSSI,FirstSeen,LastSeen,Count)
/locations.db                # GPS coordinates per device (MAC,lat1,lon1;lat2,lon2)
/device_index.idx            # Quick lookup index of known MACs
```

**Format Example (detections.db):**
```
# Flock Detection Database
# Format: MAC,Type,RSSI,FirstSeen,LastSeen,Count
AA:BB:CC:DD:EE:FF,WiFi,-65,1234567,1234890,5
11:22:33:44:55:66,BLE,-72,1234568,1234891,3
```

### Log Files (Session Logs)
```
/logs/
├── detections_YYYYMMDD_HHMMSS.log    # JSON log of all detections
└── system_YYYYMMDD_HHMMSS.log        # System events and errors
```

### Export Files (Generated on demand)
```
/export_map.geojson          # GeoJSON for OpenStreetMap visualization
/export_data.csv             # CSV export of all detections with locations
```

**GeoJSON Example:**
```json
{
  "type": "FeatureCollection",
  "features": [{
    "type": "Feature",
    "geometry": {
      "type": "Point",
      "coordinates": [-74.0060, 40.7128]
    },
    "properties": {
      "mac": "AA:BB:CC:DD:EE:FF",
      "type": "WiFi",
      "rssi": -65,
      "detections": 5
    }
  }]
}
```

---

## How Data Flows

### 1. Detection Event
```
WiFi/BLE Scanner → Detection Handler → Data Manager (in-memory cache)
```

### 2. Database Update (Every 30 seconds or when device goes out of range)
```
Data Manager → SD Card (/detections.db, /locations.db)
```

### 3. Export (Hold BOOT button for 2+ seconds)
```
Data Manager → /export_map.geojson
              → /export_data.csv
```

### 4. Viewing on Map
```
Remove SD Card → Copy export_map.geojson to computer
              → Open in:
                 - umap.openstreetmap.fr
                 - geojson.io
                 - QGIS
                 - Google My Maps
```

---

## Storage Requirements

### SD Card Size Recommendations
- **Minimum:** 512MB (stores ~100K detections)
- **Recommended:** 2-8GB (stores ~1M+ detections with locations)
- **Format:** FAT32 (required for ESP32 compatibility)

### Database Growth Estimates
- **1 detection:** ~150 bytes (database entry + location)
- **1 hour active scanning:** ~500 detections = 75 KB
- **1 day active scanning:** ~12,000 detections = 1.8 MB
- **1 month:** ~360,000 detections = 54 MB

---

## Key Features

### Fast Detection Matching
- **Known devices:** In-memory cache lookup (< 1ms)
- **New devices:** Added to cache immediately
- **Visual feedback:** Yellow flash for known, Red flash for new

### Database Persistence
- **Auto-flush:** Every 30 seconds or when detection ends
- **Crash recovery:** Database survives power loss
- **Incremental updates:** Only new data written

### Export Formats
- **GeoJSON:** Direct import to mapping tools
- **CSV:** Import to Excel, Google Sheets, analysis tools
- **Database:** Raw format for custom processing

---

## Usage Examples

### Export Data While in Field
1. Hold BOOT button for 2+ seconds
2. Purple LED flashes (exporting...)
3. Green LED flashes (complete!)
4. Files created: `/export_map.geojson`, `/export_data.csv`

### View Detections on Map
1. Remove SD card from ESP32
2. Copy `export_map.geojson` to computer
3. Upload to https://geojson.io or http://umap.openstreetmap.fr
4. See all detection points on interactive map

### Analyze in Excel
1. Copy `export_data.csv` to computer
2. Open in Excel/Google Sheets
3. Sort by detection count, filter by type, etc.

### Merge with Historical Data
1. Run PowerShell conversion script on SD card
2. Existing database merges with new detections
3. Detection counts accumulate over time

---

## Maintenance

### Clear Database (Start Fresh)
1. Remove SD card
2. Delete `/detections.db`, `/locations.db`, `/device_index.idx`
3. Re-insert SD card
4. Device starts with empty database

### Backup Data
1. Copy entire SD card contents to computer
2. Store backup folder with date
3. Can restore by copying back to SD card

### Update Detection Patterns
1. Edit `src/config/patterns.h` on computer
2. Rebuild and reflash firmware
3. Existing database remains intact
