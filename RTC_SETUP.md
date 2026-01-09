# DS3231 RTC Setup Guide

## Overview
The DS3231 is a high-precision Real-Time Clock (RTC) that provides accurate timestamps for detections even when GPS is unavailable or disabled. It includes a backup battery to maintain time during power loss.

## Hardware Wiring

### DS3231 Module Connections
Connect the DS3231 RTC module to your ESP32:

| DS3231 Pin | ESP32 Pin | Description |
|------------|-----------|-------------|
| VCC        | 3.3V      | Power supply (3.3V) |
| GND        | GND       | Ground |
| SDA        | GPIO 21   | I2C Data line |
| SCL        | GPIO 22   | I2C Clock line |

**Note:** The DS3231 module typically includes pull-up resistors on SDA/SCL, so external pull-ups are usually not needed.

## Configuration

### Enable RTC in config.json
Add or modify the `enable_rtc` setting in your `config.json` file:

```json
{
  "hardware": {
    "enable_gps": true,
    "enable_rtc": true,
    ...
  }
}
```

### RTC-GPS Synchronization
When both GPS and RTC are enabled:
- The RTC will automatically sync from GPS time once per hour
- This ensures highly accurate timestamps even when GPS signal is temporarily lost
- If GPS loses fix, the RTC continues providing accurate timestamps

### RTC-Only Mode
You can use RTC without GPS:
```json
{
  "hardware": {
    "enable_gps": false,
    "enable_rtc": true,
    ...
  }
}
```

In this mode:
- Set the RTC time manually before deployment
- Timestamps will be accurate based on the RTC's ±2ppm crystal
- No GPS dependency for timestamps

## Features

### Automatic Time Management
- **GPS Sync:** Automatically syncs from GPS when available
- **Power Loss Detection:** Detects if RTC lost power and needs time reset
- **Temperature Compensation:** DS3231 includes TCXO for temperature stability
- **Battery Backup:** Maintains time with CR2032 coin cell battery

### Timestamp Usage
When RTC is enabled and valid:
- Detection records use ISO 8601 format timestamps (YYYY-MM-DD HH:MM:SS)
- SD card file timestamps are accurate
- Export files have proper date/time stamps

When RTC is not available:
- Falls back to `millis()` relative timestamps
- Shows time as seconds since boot (e.g., "123.456s")

## Troubleshooting

### RTC Not Detected
```
ERROR: Could not find DS3231 RTC!
Check wiring: SDA->GPIO21, SCL->GPIO22
```
**Solutions:**
- Verify I2C connections (SDA to GPIO21, SCL to GPIO22)
- Check that DS3231 module is powered (3.3V)
- Ensure I2C address is 0x68 (default for DS3231)
- Try using an I2C scanner sketch to verify module is detected

### RTC Lost Power Warning
```
WARNING: RTC lost power, time may be incorrect!
RTC set to compile time as fallback
```
**Solutions:**
- Install a CR2032 battery in the DS3231 module
- Manually set time or wait for GPS sync
- Check battery contacts if battery is installed

### Invalid Time
If RTC shows year before 2020 or after 2100:
- Time is considered invalid
- System will fall back to millis() timestamps
- Sync from GPS or set time manually

## Setting Time Manually

### Via GPS
Simply enable both GPS and RTC. The system will automatically sync:
```cpp
// Happens automatically in main loop every hour
if (gpsManager.isValid()) {
    rtcManager.syncFromGPS(...);
}
```

### Via Serial (Future Feature)
Add serial command support to set time manually:
```
SET_TIME 2026-01-09 14:30:00
```

### Via Compile Time
The RTC is automatically set to compile time on first boot if no valid time exists. This provides a reasonable fallback but may be hours/days old depending on when the code was compiled.

## Battery Life
- **CR2032 Battery:** Typical RTC backup battery
- **Lifespan:** 5-10 years in backup mode (no VCC power)
- **Replacement:** Replace when voltage drops below 2.3V

## Technical Specifications
- **Accuracy:** ±2ppm (±1 minute/year)
- **Operating Voltage:** 2.3V - 5.5V (3.3V recommended for ESP32)
- **Backup Battery:** CR2032 (3V)
- **I2C Address:** 0x68
- **Temperature Range:** -40°C to +85°C
- **Temperature Sensor:** ±3°C accuracy
- **TCXO:** Temperature-Compensated Crystal Oscillator

## Library
This project uses the **Adafruit RTClib** library:
- Repository: https://github.com/adafruit/RTClib
- Documentation: https://adafruit.github.io/RTClib/html/index.html
- Version: 2.1.1+

## Benefits for This Project

1. **Accurate Detection Timestamps**
   - Every WiFi/BLE detection gets precise timestamp
   - Useful for pattern analysis and forensics

2. **GPS-Independent Operation**
   - Continue logging with accurate timestamps even without GPS signal
   - Useful in urban canyons or indoor areas

3. **Data Correlation**
   - Correlate detections across multiple devices
   - Match timestamps with external data sources

4. **File Timestamps**
   - SD card files have accurate creation/modification times
   - Export files are properly dated

5. **Low Power**
   - RTC continues running on battery backup
   - Negligible power consumption
