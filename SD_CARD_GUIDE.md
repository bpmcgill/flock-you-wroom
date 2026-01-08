# SD Card Setup Guide

This document describes the SD card structure for the FlockYouWroom detection system.

## Initial SD Card Setup

### 1. Format SD Card
- **Format:** FAT32 (required for ESP32)
- **Size:** 512MB - 32GB recommended
- **Label:** FLOCKYOU (optional)

### 2. Load Historical Data (Optional)

If you have existing detection data from the `datasets` folder:

```powershell
# Run from project root
.\convert-datasets.ps1

# Copy output files to SD card root:
detections.db
device_index.idx
locations.db
```

### 3. Insert SD Card
- Insert formatted SD card into ESP32 SD card slot
- Power on device
- Check serial monitor for: "Loaded X known devices from database"

---

## SD Card File Structure

After the device runs, your SD card will contain:

```
/                            # Root directory
├── detections.db            # Main detection database
├── device_index.idx         # Fast MAC address lookup index
├── locations.db             # GPS coordinates for each device
├── export_map.geojson       # Map export (created on button press)
├── export_data.csv          # CSV export (created on button press)
│
└── logs/                    # Session logs (if enabled)
    ├── detections_20260106_143022.log
    └── system_20260106_143022.log
```

---

## Database Files

### detections.db
Main detection database with all known devices.

**Format:**
```
# Flock Detection Database
# Format: MAC,Type,RSSI,FirstSeen,LastSeen,Count
AA:BB:CC:DD:EE:FF,WiFi,-65,1234567,1234890,5
11:22:33:44:55:66,BLE,-72,1234568,1234891,3
22:33:44:55:66:77,Raven,-80,1234569,1234892,12
```

**Fields:**
- `MAC`: Device MAC address
- `Type`: WiFi, BLE, or Raven
- `RSSI`: Last recorded signal strength (dBm)
- `FirstSeen`: First detection timestamp (milliseconds)
- `LastSeen`: Last detection timestamp (milliseconds)
- `Count`: Total number of detections

### locations.db
GPS coordinates for each detected device.

**Format:**
```
AA:BB:CC:DD:EE:FF,40.712800,-74.006000;40.712900,-74.006100
11:22:33:44:55:66,40.713000,-74.006200
```

**Fields:**
- First column: MAC address
- Remaining: Semicolon-separated lat,lon pairs

### device_index.idx
Quick lookup index of all known MAC addresses (one per line).

**Format:**
```
AA:BB:CC:DD:EE:FF
11:22:33:44:55:66
22:33:44:55:66:77
```

---

## Export Files

### export_map.geojson
GeoJSON file for viewing detections on a map.

**Usage:**
1. Copy file to computer
2. Open in:
   - https://geojson.io (instant visualization)
   - http://umap.openstreetmap.fr (create custom map)
   - QGIS (professional GIS software)
   - Google My Maps (import and share)

**Example:**
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

### export_data.csv
Spreadsheet-compatible export with all detection data.

**Usage:**
1. Copy file to computer
2. Open in Excel, Google Sheets, or LibreOffice Calc
3. Filter, sort, analyze as needed

**Format:**
```csv
MAC,Type,RSSI,FirstSeen,LastSeen,DetectionCount,Locations
AA:BB:CC:DD:EE:FF,WiFi,-65,1234567,1234890,5,"40.712800,-74.006000;40.712900,-74.006100"
```

---

## Exporting Data

### During Operation (Hold BOOT Button)
1. **Hold BOOT button** for 2+ seconds
2. **Purple LED** flashes 3 times (exporting...)
3. **Files created:**
   - `/export_map.geojson`
   - `/export_data.csv`
4. **Green LED** flashes 2 times (complete!)

### After Removing SD Card
1. Power off ESP32
2. Remove SD card
3. Insert into computer
4. Copy export files
5. View in mapping software or spreadsheet

---

## Viewing Detections on a Map

### Method 1: geojson.io (Easiest)
1. Go to https://geojson.io
2. Click "Open" → "File"
3. Select `export_map.geojson`
4. View interactive map with all detection points

### Method 2: uMap (Custom Maps)
1. Go to http://umap.openstreetmap.fr
2. Click "Create a map"
3. Click "Import data"
4. Upload `export_map.geojson`
5. Customize colors, icons, descriptions
6. Save and share URL

### Method 3: Google My Maps
1. Go to https://www.google.com/mymaps
2. Create new map
3. Click "Import"
4. Upload `export_data.csv`
5. Choose "Locations" column for coordinates
6. Customize and share

---

## Database Maintenance

### Check Database Size
```
detections.db:   ~150 bytes per device
locations.db:    ~50 bytes per location point
device_index.idx: ~18 bytes per device

Example: 1,000 devices = ~220 KB total
```

### Backup Database
**Recommended:** Weekly or after significant detection events

1. Power off device
2. Remove SD card
3. Copy entire SD card to computer folder
4. Name folder: `backup_YYYY-MM-DD`

### Restore from Backup
1. Copy files from backup folder
2. Paste to SD card root
3. Insert in ESP32
4. Device loads historical data

### Clear Database (Start Fresh)
**Warning:** This deletes all detection history!

1. Remove SD card
2. Delete these files:
   - `detections.db`
   - `locations.db`
   - `device_index.idx`
3. Re-insert SD card
4. Device starts with empty database

### Merge Multiple Databases
If you have multiple SD cards with different data:

1. Copy all SD card contents to computer
2. Run PowerShell merge script:
   ```powershell
   .\convert-datasets.ps1 -Merge
   ```
3. Copy merged database back to SD card

---

## Troubleshooting

### "SD Card Mount Failed"
- Check card is formatted as FAT32
- Try different SD card
- Check SD card pins/connections

### "Failed to open database for writing"
- SD card may be write-protected
- SD card may be full
- File system may be corrupted (reformat)

### "Loaded 0 known devices"
- Database files don't exist (normal for first run)
- Database file is empty or corrupted
- Device will create new database automatically

### Database Corruption
If database becomes corrupted:

1. Power off device
2. Backup SD card contents
3. Delete corrupted database files
4. Re-run conversion script on backup data
5. Copy new database to SD card

---

## Performance Tips

### Optimal Settings
- **Flush Interval:** 30 seconds (default)
- **Cache Size:** 500 devices (default)
- **SD Card Speed:** Class 10 or better

### When to Flush
Database is automatically flushed:
- Every 30 seconds
- When device goes out of range
- When cache reaches 500 devices
- On button press (export)

### Reducing Write Wear
- Use high-quality SD card (SanDisk, Samsung)
- Don't reduce flush interval below 30s
- Enable wear leveling (automatic on modern cards)
- Expected lifespan: 100,000+ write cycles = years of use

---

## File Size Projections

| Usage Scenario | Detections/Day | DB Size/Month | SD Card Needed |
|----------------|----------------|---------------|----------------|
| Light (home)   | 100            | 450 KB        | 512 MB         |
| Medium (urban) | 1,000          | 4.5 MB        | 1-2 GB         |
| Heavy (car)    | 10,000         | 45 MB         | 2-4 GB         |
| Continuous     | 100,000        | 450 MB        | 8-16 GB        |

*Includes database + locations + exports*
