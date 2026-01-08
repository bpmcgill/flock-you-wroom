# FlockYouWroom v2.0 - Quick Start Guide

## What's New in v2.0

### 🗄️ Persistent Detection Database
- All detections stored on SD card
- Survives power cycles and crashes
- Fast in-memory caching for instant lookups

### 🎨 Visual Feedback for Known Devices
- **New Device:** Red LED flash + 3 beeps (urgent!)
- **Known Device:** Yellow LED flash + 1 beep (less urgent)
- Easy to distinguish between first-time and repeat detections

### 🗺️ Map Export
- Hold BOOT button for 2+ seconds
- Exports GeoJSON file ready for OpenStreetMap
- View all detections on interactive map

### ⚡ Performance Improvements
- **BLE:** 5x longer scans, 50x faster intervals (near-continuous)
- **WiFi:** 2.5x faster channel hopping with adaptive priority channels
- **Dual-Core:** BLE on Core 0, WiFi on Core 1 (true parallel scanning)
- **RSSI Filtering:** Ignores weak signals below -85 dBm

---

## Quick Start

### 1. Hardware Setup
```
ESP32 DevKit V4
├── LED Strip (Pin 5) - 4x WS2812B [OPTIONAL]
├── Buzzer (Pin 23) - Active 5V OR Passive 3.3V/5V [OPTIONAL]
├── OLED Display (I2C: SDA=21, SCL=22) [OPTIONAL]
├── GPS Module (UART: RX=16, TX=17) [OPTIONAL]
└── SD Card (SPI: CS=15, MOSI=13, MISO=12, SCK=14) [OPTIONAL]

Note: All hardware is optional and can be disabled in config.json
```

### 2. SD Card Preparation
1. Format as **FAT32** (FAT16/exFAT also supported)
2. **Required:** Copy `config.json.example` to SD card as `config.json`
   - Configure which hardware you have installed
   - Set `enable_gps`, `enable_leds`, `enable_buzzer`, `enable_oled`, `enable_sd_card`
   - Set `buzzer_is_passive: true` for 3-pin PWM buzzer, `false` for 2-pin active
   - Adjust scan parameters if needed (channel hop, BLE intervals, RSSI threshold)
3. (Optional) Load historical data:
   ```powershell
   .\convert-datasets.ps1
   ```
4. Copy generated `detections.db`, `locations.db`, `device_index.idx` to SD card root
5. Insert SD card into ESP32

### 3. First Boot
```
Power on → Device loads known devices → Starts scanning
```

Serial output shows:
```
Loaded 127 known devices from database
System ready - hunting for Flock Safety devices...
```

---

## Daily Usage

### Normal Operation
- **Green LED pulse:** Scanning (no detections)
- **Red LED flash + 3 beeps:** New device detected!
- **Yellow LED flash + 1 beep:** Known device re-detected
- **Display:** Shows detection count, GPS status, SD card status

### Exporting Data
1. **Hold BOOT button** for 2+ seconds
2. **Purple flashes:** Exporting...
3. **Green flashes:** Done!
4. Files created:
   - `/export_map.geojson` - For mapping
   - `/export_data.csv` - For spreadsheets

### Viewing on Map
1. Remove SD card
2. Copy `export_map.geojson` to computer  
3. Open in https://geojson.io
4. See all detection points on map!

---

## File Organization

### Embedded in ESP32 (Firmware)
```
src/config/pins.h          - Hardware configuration
src/config/patterns.h      - Detection patterns (MAC, SSID, UUID)
src/main.cpp               - Main program
src/detection/*            - WiFi/BLE/Raven detectors
src/hardware/*             - LED, buzzer, display, GPS, data manager
```

### Stored on SD Card (Runtime)
```
/config.json               - Hardware & scan configuration
/detections.db             - Main database
/device_index.idx          - Quick lookup index
/locations.db              - GPS coordinates
/export_YYYYMMDD_HHMMSS.geojson  - Map export (on demand)
/export_YYYYMMDD_HHMMSS.csv      - CSV export (on demand)
```

---

## Visual Indicators

### LED Colors
| Color  | Meaning                          |
|--------|----------------------------------|
| Green  | Scanning (breathing effect)      |
| Red    | New device detected (flash 3x)   |
| Yellow | Known device (flash 1x)          |
| Orange | Heartbeat (device still in range)|
| Blue   | Exporting data / WiFi detection  |
| Purple | BLE detection                    |
| White  | Raven detected (critical!)       |

### Buzzer Patterns

**Active Buzzer (2-pin):**
| Pattern        | Meaning                    |
|----------------|----------------------------|
| 2 beeps (boot) | System startup complete    |
| 3 fast beeps   | New device alert           |
| 1 short beep   | Known device re-detected   |
| 2 slow beeps   | Heartbeat (still tracking) |

**Passive Buzzer (3-pin PWM):**
| Pattern               | Meaning                    |
|-----------------------|----------------------------|
| C4-E4-G4-C5 scale     | System startup complete    |
| 2500Hz high tone (3x) | New device alert           |
| 1500Hz low tone (1x)  | Known device re-detected   |
| 1800Hz dual-tone      | Heartbeat (still tracking) |

Set `buzzer_is_passive: true` in config.json for passive buzzer.

---

## Detection Flow

```
┌─────────────────────────────────────────────────────────┐
│ WiFi/BLE Scanner detects signal                         │
└────────────────────┬────────────────────────────────────┘
                     │
                     ↓
┌─────────────────────────────────────────────────────────┐
│ Check patterns: MAC prefix, SSID, UUID                  │
└────────────────────┬────────────────────────────────────┘
                     │
                     ↓ Match!
┌─────────────────────────────────────────────────────────┐
│ Data Manager: Check if device known                     │
│ - Search in-memory cache (< 1ms)                        │
│ - If not found, add to cache as NEW                     │
└────────────────────┬────────────────────────────────────┘
                     │
        ┌────────────┴────────────┐
        ↓                         ↓
┌──────────────┐          ┌──────────────┐
│ Known Device │          │  New Device  │
├──────────────┤          ├──────────────┤
│ Yellow LED   │          │ Red LED      │
│ 1 short beep │          │ 3 fast beeps │
│ Update count │          │ Add to DB    │
└──────────────┘          └──────────────┘
        │                         │
        └────────────┬────────────┘
                     │
                     ↓
┌─────────────────────────────────────────────────────────┐
│ Add GPS location, log to SD, update display             │
└─────────────────────────────────────────────────────────┘
```

---

## Performance Specs

### Scanning Speed
- **BLE Scan:** 5 seconds every 100ms (95% duty cycle)
- **WiFi Hop:** 200ms per channel (13 channels in 2.6s)
- **Full Cycle:** Both radios scanning simultaneously
- **Detection Latency:** < 3 seconds typical

### Database Performance
- **Known device lookup:** < 1ms (in-memory)
- **New device add:** < 5ms (cache only)
- **Database flush:** 30s intervals (500KB/s write speed)
- **Cache size:** 500 devices in RAM

### Power Consumption
- **Scanning:** ~180mA @ 5V
- **Detection:** ~200mA @ 5V (LEDs on)
- **Idle:** ~150mA @ 5V

---

## Common Tasks

### View Detection Stats
```
Check OLED display:
- Line 1: Total detections
- Line 2: WiFi vs BLE count
- Line 3: GPS status
- Line 4: SD card status
```

### Export for Analysis
```powershell
# On computer with SD card
1. Open export_data.csv in Excel
2. Create pivot table
3. Group by Type, MAC, or Location
4. Generate charts and reports
```

### Map Detections
```
1. Upload export_map.geojson to uMap
2. Customize markers by detection type
3. Add heatmap layer for density
4. Share map URL with others
```

### Backup Database
```
Monthly backup recommended:
1. Copy entire SD card to dated folder
2. Keep 3 most recent backups
3. Expected size: ~50MB per month (heavy use)
```

---

## Troubleshooting

### No Detections
- Check serial output for scanning messages
- Verify patterns in `src/config/patterns.h`
- Ensure WiFi/BLE antennas not blocked
- Try areas with known surveillance cameras

### "SD Card Mount Failed"
- Reformat SD card as FAT32
- Try different SD card (Class 10 recommended)
- Check SD card socket connections

### Known Devices Show as New
- Database not loaded (check serial: "Loaded X devices")
- Wrong MAC address format in database
- Cache cleared (normal after restart)

### LEDs Not Working
- Check LED strip power (5V)
- Verify pin connection (Pin 5)
- Check LED_COUNT in pins.h matches your strip
- Disable in config.json if not installed: `"enable_leds": false`

### Buzzer Not Working or Wrong Sound
- **Active buzzer (2-pin)**: Set `"buzzer_is_passive": false`
- **Passive buzzer (3-pin)**: Set `"buzzer_is_passive": true`
- Check connection to GPIO23
- Verify power (active=5V, passive=3.3V or 5V)
- Disable in config.json if not installed: `"enable_buzzer": false`

### Config.json Not Loading
- Verify file named exactly `config.json` (not .txt)
- File must be in SD card root directory
- Validate JSON syntax at https://jsonlint.com
- Check serial: "Loading settings from config.json" should appear

---

## Advanced Features

### Convert Legacy Data
```powershell
# Merge old CSV/JSON logs with current database
.\convert-datasets.ps1 -Merge

# Preview files before converting
.\convert-datasets.ps1 -ShowSample
```

### Tune Scan Parameters
Edit `/config.json` on SD card:
```json
{
  "scan": {
    "channel_hop_interval": 200,   // WiFi hop speed (ms)
    "ble_scan_duration": 5,        // BLE scan time (s)
    "ble_scan_interval": 100,      // BLE gap (ms)
    "rssi_threshold": -85,         // Signal strength filter (dBm)
    "detection_cooldown": 2000     // Minimum gap between alerts (ms)
  }
}
```

OR edit firmware defaults in `src/config/pins.h` (requires reflashing):
```cpp
#define CHANNEL_HOP_INTERVAL  200   // WiFi hop speed (ms)
#define BLE_SCAN_DURATION     5     // BLE scan time (s)
#define BLE_SCAN_INTERVAL     100   // BLE gap (ms)
```

### Custom Detection Patterns
Edit `src/config/patterns.h`:
```cpp
// Add MAC prefixes
const char* mac_prefixes[] = {
    "aa:bb:cc",  // Your custom prefix
    // ... existing entries
};

// Add SSID patterns
const char* wifi_ssid_patterns[] = {
    "YourPattern",  // Case-insensitive
    // ... existing entries
};
```

---

## Support & Documentation

- **Configuration:** See `CONFIGURATION.md` for hardware enable/disable
- **File Structure:** See `FILE_STRUCTURE.md`
- **SD Card Guide:** See `SD_CARD_GUIDE.md`
- **Dataset Conversion:** See `datasets/README.md`
- **Serial Monitor:** 115200 baud for debug output

---

## System Requirements

### ESP32
- **Board:** ESP32-WROOM-32 DevKit V4
- **Flash:** 4MB minimum
- **RAM:** 520KB SRAM
- **Clock:** 240MHz dual-core

### SD Card
- **Format:** FAT32
- **Size:** 512MB - 32GB
- **Speed:** Class 10 recommended
- **Wear:** 100K+ cycles (years of use)

### Power
- **USB:** 5V 500mA minimum
- **Battery:** 5V 1A recommended (for GPS + SD writes)
- **Runtime:** ~10 hours on 5000mAh battery

---

## Success Indicators

**System is working correctly when you see:**
1. ✅ Serial: "Loaded X known devices from database"
2. ✅ Green LED breathing effect while scanning
3. ✅ OLED display showing scan status
4. ✅ Red flash + beeps when device detected
5. ✅ Serial: "Known device re-detected" on repeat detections
6. ✅ Export files created when button held
