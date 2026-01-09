# Flock You: Flock Safety Detection System

<img src="flock.png" alt="Flock You" width="300px">

**Professional surveillance camera detection for the Oui-Spy device available at [colonelpanic.tech](https://colonelpanic.tech)**

## Overview

Flock You is an advanced detection system designed to identify Flock Safety surveillance cameras, Raven gunshot detectors, and similar surveillance devices using multiple detection methodologies. Built for ESP32-WROOM-32 and Xiao ESP32 S3 microcontrollers, it provides real-time monitoring with audio/visual alerts, persistent database tracking, and comprehensive JSON output. The system features dual-core parallel scanning, GPS location tracking, and specialized BLE service UUID fingerprinting for detecting SoundThinking/ShotSpotter Raven acoustic surveillance devices.

## Features

### Multi-Method Detection
- **WiFi Promiscuous Mode**: Captures probe requests and beacon frames
- **Bluetooth Low Energy (BLE) Scanning**: Monitors BLE advertisements
- **MAC Address Filtering**: Detects devices by known MAC prefixes
- **SSID Pattern Matching**: Identifies networks by specific names
- **Device Name Pattern Matching**: Detects BLE devices by advertised names
- **BLE Service UUID Detection**: Identifies Raven gunshot detectors by service UUIDs (NEW)

### Dual-Core Architecture (ESP32-WROOM-32)
- **Parallel Scanning**: BLE on Core 0, WiFi on Core 1 for maximum efficiency
- **True Simultaneous Operation**: No missed detections during channel hopping
- **Optimized Performance**: 5s BLE scans with 100ms intervals, 200ms WiFi channel hops
- **95% BLE Duty Cycle**: Near-continuous BLE monitoring (49ms scan window, 50ms interval)

### Persistent Detection Database (ESP32-WROOM-32)
- **In-Memory Cache**: Fast lookup with HashMap (500 device capacity)
- **SD Card Storage**: Detections, locations, and device index files
- **Known Device Detection**: Yellow LED alert for re-detected devices vs red for new
- **GPS Tracking**: Multiple location records per device
- **GeoJSON Export**: OpenStreetMap-compatible format (BOOT button 2s hold)
- **CSV Export**: Spreadsheet-compatible detection logs
- **Auto-Flush**: Database writes every 30 seconds

### Configuration System (ESP32-WROOM-32)
- **config.json**: SD card-based hardware and scanning configuration
- **Hardware Toggle**: Enable/disable GPS, LEDs, buzzer, OLED, SD card
- **Scan Tuning**: Adjust channel hop speed, BLE intervals, RSSI threshold
- **Passive Buzzer Support**: Musical tones via PWM (1500Hz known, 2500Hz new)
- **LED Modes**: 6 pre-built modes (Unified, Status, Signal, Counter, Threat, Custom)
- **Individual LED Control**: Assign specific functions to each of 4 LEDs
- **Runtime Adaptation**: System adapts to missing hardware components

See [CONFIGURATION.md](CONFIGURATION.md) for complete details and [LED_MODES.md](LED_MODES.md) for LED configuration guide.

### Enhanced Visual & Audio Feedback System (ESP32-WROOM-32)
- **LED Animations**: 4x WS2812B RGB LEDs with color-coded alerts
  - **Boot Sequence**: Blue LED fade-in animation
  - **Scanning Mode**: Slow green pulse (breathing effect)
  - **WiFi Detection**: Blue flash
  - **BLE Detection**: Purple flash
  - **Detection Alert**: Fast red flashing (3 times) for NEW devices
  - **Known Device Alert**: Yellow flash (1 time) for re-detected devices
  - **Raven Detection**: Red + white strobe (critical threat)
  - **Heartbeat Pulse**: Orange pulse every 10 seconds while device in range
- **Buzzer Alerts**: Active or passive buzzer support
  - **Active Buzzer** (2-pin): Simple on/off beeping
    - Boot: 2 beeps, Detection: 3 beeps, Heartbeat: 2 beeps
  - **Passive Buzzer** (3-pin PWM): Musical tones
    - Boot: C4-E4-G4-C5 musical scale
    - New Device: 2500Hz high-pitched tone (3 beeps)
    - Known Device: 1500Hz lower tone (1 beep)
    - Heartbeat: 1800Hz dual-tone pulse
- **OLED Display**: 128x64 SSD1315 display with real-time status
  - Detection count (WiFi + BLE)
  - GPS coordinates and status
  - SD card status
  - Signal strength (RSSI)
  - Current scanning status

### GPS Location Tracking (ESP32-WROOM-32)
- **Real-time GPS**: GY-NEO6Mv2 GPS module for location tracking
- **Coordinate Logging**: All detections include GPS latitude/longitude
- **Altitude Tracking**: Records altitude in meters
- **Satellite Count**: Displays number of satellites in view
- **Fix Status**: Shows GPS fix status (FIX, SEARCHING, NO_GPS)

### Real-Time Clock Support (ESP32-WROOM-32)
- **DS3231 RTC Module**: High-precision timestamps (±2ppm accuracy)
- **GPS Auto-Sync**: Automatically syncs from GPS every hour
- **Battery Backup**: CR2032 maintains time during power loss
- **GPS-Independent**: Accurate timestamps even without GPS signal
- **Temperature Compensated**: TCXO for stability across temperature ranges
- See [RTC_SETUP.md](RTC_SETUP.md) for wiring and configuration

### SD Card Data Logging (ESP32-WROOM-32)
- **CSV Log Files**: Automatic logging to MicroSD card
- **Detection Records**: Timestamp, protocol, MAC, RSSI, GPS coordinates
- **Persistent Storage**: Data survives power cycles
- **Accurate Timestamps**: Uses RTC when available, falls back to millis()

### Audio Alert System (Xiao ESP32 S3)
- **Boot Sequence**: 2 beeps (low pitch → high pitch) on startup
- **Detection Alert**: 3 fast high-pitch beeps when device detected
- **Heartbeat Pulse**: 2 beeps every 10 seconds while device remains in range
- **Range Monitoring**: Automatic detection of device leaving range

### Comprehensive Output
- **JSON Detection Data**: Structured output with timestamps, RSSI, MAC addresses
- **Real-time Web Dashboard**: Live monitoring at `http://localhost:5000`
- **Serial Terminal**: Real-time device output in the web interface
- **Detection History**: Persistent storage and export capabilities (CSV, KML)
- **Device Information**: Full device details including signal strength and threat assessment
- **Detection Method Tracking**: Identifies which detection method triggered the alert

## Hardware Requirements

### Option 1: ESP32-WROOM-32 DevKit V4 (Enhanced System)
- **Microcontroller**: ESP32-WROOM-32 on ESP32 DevKit V4 board
- **LED System**: 4x WS2812B RGB LEDs (addressable LED strip)
- **Display**: SSD1315 OLED Display (128x64, Blue/Yellow, I2C)
- **RTC**: DS3231 Real-Time Clock Module (I2C, optional)
- **GPS Module**: GY-NEO6Mv2 GPS Module (UART)
- **Storage**: MicroSD Card Module (SPI)
- **Audio**: 2-pin 5V Active Buzzer
- **Power**: USB for programming and power, or external 5V power supply
- **Connectivity**: Micro-USB or USB-C (depending on DevKit version)

#### Pin Mapping for ESP32 DevKit V4
```
Component          | ESP32 GPIO | Notes
-------------------|------------|----------------------------------
WS2812B LEDs       | GPIO 5     | Data line (4 LEDs)
Buzzer             | GPIO 23    | 2-pin active OR 3-pin passive
OLED SDA           | GPIO 21    | I2C Data (SSD1315)
OLED SCL           | GPIO 22    | I2C Clock (SSD1315)
RTC SDA            | GPIO 21    | I2C Data (DS3231, shared w/ OLED)
RTC SCL            | GPIO 22    | I2C Clock (DS3231, shared w/ OLED)
GPS TX             | GPIO 16    | UART2 RX (GPS transmit)
GPS RX             | GPIO 17    | UART2 TX (GPS receive)
SD Card CS         | GPIO 15    | SPI Chip Select
SD Card MOSI       | GPIO 13    | SPI MOSI (HSPI)
SD Card MISO       | GPIO 12    | SPI MISO (HSPI)
SD Card SCK        | GPIO 14    | SPI Clock (HSPI)
BOOT Button        | GPIO 0     | Export database (hold 2 seconds)
```

**Note:** OLED and RTC share the I2C bus (GPIO 21/22) with different addresses (0x3C and 0x68).

**For complete wiring diagrams and troubleshooting, see [WIRING.md](WIRING.md)**

#### Complete Wiring Diagram for ESP32-WROOM-32 Setup

**WS2812B LED Strip Connection:**
```
ESP32 DevKit V4           WS2812B LED Strip
┌─────────────┐          ┌──────────────┐
│   GPIO 5    │─────────>│ DIN (Data)   │
│   5V        │─────────>│ VCC (+5V)    │
│   GND       │─────────>│ GND          │
└─────────────┘          └──────────────┘
```

**SSD1315 OLED Display Connection:**
```
ESP32 DevKit V4           SSD1315 OLED (128x64)
┌─────────────┐          ┌──────────────┐
│ GPIO21(SDA) │<────────>│ SDA          │
│ GPIO22(SCL) │─────────>│ SCL          │
│   3.3V      │─────────>│ VCC          │
│   GND       │─────────>│ GND          │
└─────────────┘          └──────────────┘
```

**GY-NEO6Mv2 GPS Module Connection:**
```
ESP32 DevKit V4           GY-NEO6Mv2 GPS
┌─────────────┐          ┌──────────────┐
│ GPIO16(RX2) │<─────────│ TX           │
│ GPIO17(TX2) │─────────>│ RX           │
│ 3.3V or 5V  │─────────>│ VCC          │
│   GND       │─────────>│ GND          │
└─────────────┘          └──────────────┘
Note: Most GPS modules work with both 3.3V and 5V
```

**MicroSD Card Module Connection:**
```
ESP32 DevKit V4           SD Card Module
┌─────────────┐          ┌──────────────┐
│  GPIO 15    │─────────>│ CS           │
│  GPIO 13    │─────────>│ MOSI         │
│  GPIO 12    │<─────────│ MISO         │
│  GPIO 14    │─────────>│ SCK          │
│   5V        │─────────>│ VCC          │
│   GND       │─────────>│ GND          │
└─────────────┘          └──────────────┘
```

**Active Buzzer Connection (2-pin):**
```
ESP32 DevKit V4           Active Buzzer (5V)
┌─────────────┐          ┌──────────────┐
│  GPIO 23    │─────────>│ Positive (+) │
│   GND       │─────────>│ Negative (-) │
└─────────────┘          └──────────────┘
Note: Set buzzer_is_passive: false in config.json
```

**Passive Buzzer Connection (3-pin PWM):**
```
ESP32 DevKit V4           Passive Buzzer
┌─────────────┐          ┌──────────────┐
│  GPIO 23    │─────────>│ I/O (Signal) │
│ 3.3V or 5V  │─────────>│ VCC          │
│   GND       │─────────>│ GND          │
└─────────────┘          └──────────────┘
Note: Set buzzer_is_passive: true in config.json
      5V recommended for louder volume
```

**Complete System Wiring Overview:**
```
                           ┌──────────────────────┐
                           │  ESP32 DevKit V4     │
                           │                      │
                      ┌────│ 5V    GND     3.3V   │────┐
                      │    │                      │    │
         ┌────────────┼────│ GPIO Pins:           │────┼──────┐
         │            │    │  5, 12-17, 21-23     │    │      │
         │            │    └──────────────────────┘    │      │
         │            │                                │      │
    ┌────▼──┐     ┌───▼────┐    ┌────────┐      ┌──────▼──┐   │
    │ LEDs  │     │  GPS   │    │  OLED  │      │   SD    │   │
    │(GPIO5)│     │(16,17) │    │(21,22) │      │(12-15)  │   │
    └───────┘     └────────┘    └───┬────┘      └─────────┘   │
                                     │                        │
                               ┌─────▼────┐             ┌─────▼──┐
                               │   RTC    │             │ Buzzer │
                               │ (21,22)  │             │(GPIO23)│
                               │ Shared   │             └────────┘
                               └──────────┘
```

**See [WIRING.md](WIRING.md) for detailed component connections and troubleshooting.**

## Quick Reference Guides

- **[WIRING.md](WIRING.md)** - Complete wiring diagrams, pin mappings, and troubleshooting
- **[QUICK_START.md](QUICK_START.md)** - Fast setup guide for getting started
- **[CONFIGURATION.md](CONFIGURATION.md)** - Hardware and scanning configuration via config.json
- **[LED_MODES.md](LED_MODES.md)** - LED behavior modes and customization
- **[RTC_SETUP.md](RTC_SETUP.md)** - Real-Time Clock setup and GPS synchronization
- **[SD_CARD_GUIDE.md](SD_CARD_GUIDE.md)** - SD card setup and database management
- **[FILE_STRUCTURE.md](FILE_STRUCTURE.md)** - Project organization and code structure

---

### Option 2: Oui-Spy Device (Available at colonelpanic.tech)
- **Microcontroller**: Xiao ESP32 S3
- **Wireless**: Dual WiFi/BLE scanning capabilities
- **Audio**: Built-in buzzer system
- **Connectivity**: USB-C for programming and power

### Option 3: Standard Xiao ESP32 S3 Setup
- **Microcontroller**: Xiao ESP32 S3 board
- **Buzzer**: 3V buzzer connected to GPIO3 (D2)
- **Power**: USB-C cable for programming and power

### Wiring for Standard Setup
```
Xiao ESP32 S3    Buzzer
GPIO3 (D2)  ---> Positive (+)
GND         ---> Negative (-)
```

## Installation

### Development Environment Options

You can use any of these IDEs/tools to program the ESP32-WROOM-32:

#### Option 1: PlatformIO (Recommended)
- **PlatformIO IDE** (VS Code extension) - Best for beginners
  - Install VS Code: https://code.visualstudio.com/
  - Install PlatformIO extension from VS Code marketplace
  - Open the project folder in VS Code
  - Use the PlatformIO toolbar to build and upload

- **PlatformIO Core** (Command Line)
  - Install: `pip install platformio`
  - Build: `pio run -e esp32dev`
  - Upload: `pio run -e esp32dev --target upload`
  - Monitor: `pio device monitor -e esp32dev`

#### Option 2: Arduino IDE
- Download Arduino IDE 2.x: https://www.arduino.cc/en/software
- Install ESP32 board support:
  - Go to File → Preferences
  - Add to "Additional Board Manager URLs": 
    `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
  - Go to Tools → Board → Boards Manager
  - Search "esp32" and install "ESP32 by Espressif Systems"
- Install required libraries via Library Manager:
  - NimBLE-Arduino
  - ArduinoJson
  - Adafruit NeoPixel
  - Adafruit SSD1306
  - TinyGPSPlus
  - SdFat
- Select board: Tools → Board → ESP32 Arduino → ESP32 Dev Module
- Open `src/main.cpp` and upload

### Prerequisites
- One of the IDEs listed above (PlatformIO recommended)
- Python 3.8+ (optional, for web interface)
- USB cable (Micro-USB or USB-C depending on your ESP32 DevKit)
- ESP32-WROOM-32 DevKit V4 board and components listed above

### Setup Instructions

#### For ESP32-WROOM-32 DevKit V4:
1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd flock-you-wroom
   ```

2. **Connect your ESP32 DevKit V4** via USB

3. **Flash the firmware**:
   ```bash
   pio run -e esp32dev --target upload
   ```

4. **Monitor serial output**:
   ```bash
   pio device monitor -e esp32dev
   ```

#### For Xiao ESP32 S3 / Oui-Spy Device:
1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd flock-you-wroom
   ```

2. **Connect your device** via USB-C

3. **Flash the firmware**:
   ```bash
   pio run -e xiao_esp32s3 --target upload
   ```

4. **Monitor serial output**:
   ```bash
   pio device monitor -e xiao_esp32s3
   ```

#### Web Interface Setup (Optional):
4. **Set up the web interface** (all platforms):
   ```bash
   cd api
   python3 -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   pip install -r requirements.txt
   ```

5. **Start the web server**:
   ```bash
   python flockyou.py
   ```

6. **Access the dashboard**:
   - Open your browser to `http://localhost:5000`
   - The web interface provides real-time detection monitoring
   - Serial terminal for device output
   - Detection history and export capabilities

7. **Monitor device output** (optional):
   ```bash
   pio device monitor
   ```

## Detection Coverage

### WiFi Detection Methods
- **Probe Requests**: Captures devices actively searching for networks
- **Beacon Frames**: Monitors network advertisements
- **Channel Hopping**: Cycles through all 13 WiFi channels (2.4GHz)
- **SSID Patterns**: Detects networks with "flock", "Penguin", "Pigvision" patterns
- **MAC Prefixes**: Identifies devices by manufacturer MAC addresses

### BLE Detection Methods
- **Advertisement Scanning**: Monitors BLE device broadcasts
- **Device Names**: Matches against known surveillance device names
- **MAC Address Filtering**: Detects devices by BLE MAC prefixes
- **Service UUID Detection**: Identifies Raven devices by advertised service UUIDs
- **Firmware Version Estimation**: Automatically determines Raven firmware version (1.1.x, 1.2.x, 1.3.x)
- **Active Scanning**: Continuous monitoring with 100ms intervals

### Real-World Database Integration
Detection patterns are derived from actual field data including:
- Flock Safety camera signatures
- Penguin surveillance device patterns
- Pigvision system identifiers
- Raven acoustic gunshot detection devices (SoundThinking/ShotSpotter)
- Extended battery and external antenna configurations

**Datasets from deflock.me are included in the `datasets/` folder of this repository**, providing comprehensive device signatures and detection patterns for enhanced accuracy.

### Raven Gunshot Detection System
Flock You now includes specialized detection for **Raven acoustic gunshot detection devices** (by SoundThinking/ShotSpotter) using BLE service UUID fingerprinting:

#### Detected Raven Services
- **Device Information Service** (`0000180a-...`) - Serial number, model, firmware version
- **GPS Location Service** (`00003100-...`) - Real-time device coordinates
- **Power Management Service** (`00003200-...`) - Battery and solar panel status
- **Network Status Service** (`00003300-...`) - LTE and WiFi connectivity information
- **Upload Statistics Service** (`00003400-...`) - Data transmission metrics
- **Error/Failure Service** (`00003500-...`) - System diagnostics and error logs
- **Legacy Services** (`00001809-...`, `00001819-...`) - Older firmware versions (1.1.x)

#### Firmware Version Detection
The system automatically identifies Raven firmware versions based on advertised services:
- **1.1.x (Legacy)**: Uses Health Thermometer and Location/Navigation services
- **1.2.x**: Introduces GPS, Power, and Network services
- **1.3.x (Latest)**: Full suite of diagnostic and monitoring services

#### Raven Detection Output
When a Raven device is detected, the system provides:
- Device type identification: `RAVEN_GUNSHOT_DETECTOR`
- Manufacturer: `SoundThinking/ShotSpotter`
- Complete list of advertised service UUIDs
- Service descriptions (GPS, Battery, Network status, etc.)
- Estimated firmware version
- Threat level: `CRITICAL` with score of 100

**Configuration data sourced from `raven_configurations.json`** (provided by [GainSec](https://github.com/GainSec)) in the datasets folder, containing verified service UUIDs from firmware versions 1.1.7, 1.2.0, and 1.3.1.

## Technical Specifications

### ESP32-WROOM-32 DevKit V4
- **Flash Memory**: 4MB
- **SRAM**: 520KB
- **PSRAM**: None
- **Cores**: Dual-core (Core 0: BLE, Core 1: WiFi + main loop)
- **WiFi**: 2.4GHz 802.11 b/g/n
- **Bluetooth**: BLE 4.2
- **I2C**: Hardware I2C for OLED
- **SPI**: Hardware HSPI for SD card
- **UART**: UART0 (USB serial), UART2 (GPS)
- **GPIO**: 34 pins available
- **Power**: 3.3V logic, 5V USB input
- **SD Card**: FAT16/FAT32/exFAT support (auto-detected)
- **Database**: In-memory HashMap with 500 device capacity
- **Export Formats**: GeoJSON, CSV

### Xiao ESP32 S3
- **Flash Memory**: 8MB
- **PSRAM**: 8MB OPI
- **WiFi**: 2.4GHz 802.11 b/g/n
- **Bluetooth**: BLE 5.0
- **USB**: Native USB CDC

### WiFi Capabilities
- **Frequency**: 2.4GHz only (13 channels)
- **Mode**: Promiscuous monitoring
- **Channel Hopping**: 200ms per channel (2.6s full cycle)
- **Adaptive Hopping**: Priority channels 1, 6, 11 (extra time when idle)
- **RSSI Filtering**: -85dBm threshold (configurable)
- **Packet Types**: Probe requests (0x04) and beacons (0x08)
- **Core Assignment**: WiFi runs on Core 1

### BLE Capabilities
- **Framework**: NimBLE-Arduino
- **Scan Mode**: Active scanning
- **Duration**: 5 seconds per scan
- **Interval**: 50ms between scan starts (configurable: 100ms default)
- **Window**: 49ms scan window (95% duty cycle)
- **Detection**: MAC prefix, name patterns, Raven service UUIDs
- **Core Assignment**: BLE runs on Core 0 (dedicated task)

### Audio System
- **Boot Sequence**: 200Hz → 800Hz (300ms each)
- **Detection Alert**: 1000Hz × 3 beeps (150ms each)
- **Heartbeat**: 600Hz × 2 beeps (100ms each, 100ms gap)
- **Frequency**: Every 10 seconds while device in range

### JSON Output Format

#### WiFi Detection Example (ESP32-WROOM-32)
```json
{
  "timestamp": 12345,
  "detection_time": "12.345s",
  "protocol": "wifi",
  "detection_method": "probe_request",
  "alert_level": "HIGH",
  "device_category": "FLOCK_SAFETY",
  "ssid": "Flock_Camera_001",
  "rssi": -65,
  "signal_strength": "MEDIUM",
  "channel": 6,
  "mac_address": "aa:bb:cc:dd:ee:ff",
  "gps_latitude": 34.052235,
  "gps_longitude": -118.243683,
  "gps_altitude": 100.5,
  "gps_satellites": 8,
  "threat_score": 95,
  "matched_patterns": ["ssid_pattern", "mac_prefix"],
  "device_info": {
    "manufacturer": "Flock Safety",
    "model": "Surveillance Camera",
    "capabilities": ["video", "audio", "gps"]
  }
}
```

#### Raven BLE Detection Example (ESP32-WROOM-32)
```json
{
  "timestamp": 23456,
  "detection_time": "23.456s",
  "protocol": "bluetooth_le",
  "detection_method": "raven_service_uuid",
  "device_type": "RAVEN_GUNSHOT_DETECTOR",
  "manufacturer": "SoundThinking/ShotSpotter",
  "mac_address": "12:34:56:78:9a:bc",
  "rssi": -72,
  "signal_strength": "MEDIUM",
  "device_name": "Raven-Device-001",
  "gps_latitude": 34.052235,
  "gps_longitude": -118.243683,
  "gps_altitude": 100.5,
  "gps_satellites": 8,
  "raven_service_uuid": "00003100-0000-1000-8000-00805f9b34fb",
  "raven_service_description": "GPS Location Service (Lat/Lon/Alt)",
  "raven_firmware_version": "1.3.x (Latest)",
  "threat_level": "CRITICAL",
  "threat_score": 100,
  "service_uuids": [
    "0000180a-0000-1000-8000-00805f9b34fb",
    "00003100-0000-1000-8000-00805f9b34fb",
    "00003200-0000-1000-8000-00805f9b34fb",
    "00003300-0000-1000-8000-00805f9b34fb",
    "00003400-0000-1000-8000-00805f9b34fb",
    "00003500-0000-1000-8000-00805f9b34fb"
  ]
}
```

## Usage

### Startup Sequence

#### ESP32-WROOM-32 DevKit V4:
1. **Power on** the device via USB
2. **Configuration loaded** from SD card `/config.json` (or defaults if missing)
3. **Watch the OLED display** show boot screen
4. **Listen for boot sequence**:
   - Active buzzer: 2 beeps
   - Passive buzzer: C4-E4-G4-C5 musical scale
   - Blue LED fade-in animation
5. **Wait for initialization**:
   - LED strip initializes (all LEDs briefly light up)
   - OLED displays "FLOCK DETECTOR v2.0"
   - GPS module starts acquiring satellites (if enabled)
   - SD card mounts and loads database (if enabled)
   - Database cache initialized (500 device capacity)
6. **Check OLED status**:
   - Status should show "SCANNING"
   - GPS status will show "SEARCHING" then "FIX" when locked
   - SD card shows "OK" or "ERR"
   - Detection count shows WiFi/BLE totals
7. **Dual-core scanning starts**:
   - Core 0: BLE scanning (5s scans, 100ms intervals)
   - Core 1: WiFi channel hopping (200ms per channel)
8. **Green breathing LEDs** indicate active scanning mode
9. **Export database**: Hold BOOT button (GPIO 0) for 2 seconds to export GeoJSON/CSV

#### Xiao ESP32 S3 / Oui-Spy Device:
1. **Power on** the Oui-Spy device
2. **Listen for boot beeps** (low → high pitch)
3. **Start the web server** (optional): `python flockyou.py` (from the `api` directory)
4. **Open the dashboard** (optional): Navigate to `http://localhost:5000`
5. **Connect devices** (optional): Use the web interface to connect your Flock You device and GPS
6. **System ready** when "hunting for Flock Safety devices" appears in the serial terminal

### Detection Monitoring

#### ESP32-WROOM-32 DevKit V4:
- **OLED Display**: Real-time detection count, GPS coordinates, SD status
- **LED Indicators**: Color-coded visual feedback
  - Green breathing: Scanning mode
  - Blue flash: WiFi detection
  - Purple flash: BLE detection
  - **Red flash (3x)**: NEW device detected (first time)
  - **Yellow flash (1x)**: KNOWN device re-detected
  - Red/white strobe: Raven gunshot detector (critical threat)
  - Orange pulse: Heartbeat (device still in range)
- **Audio Alerts**: 
  - Active buzzer: 3 beeps (new), 1 beep (known)
  - Passive buzzer: 2500Hz tone (new), 1500Hz tone (known)
- **Database System**:
  - In-memory cache with HashMap lookup (<1ms)
  - Persistent storage: `/detections.db`, `/locations.db`, `/device_index.idx`
  - Auto-flush every 30 seconds
  - Known device detection with visual/audio feedback
- **SD Card Logging**: All detections logged to CSV file with GPS coordinates
- **Export Function**: Hold BOOT button 2s to export GeoJSON (OpenStreetMap) and CSV
- **Serial Output**: Full JSON detection data via USB serial

#### Xiao ESP32 S3 / Oui-Spy:
- **Web Dashboard**: Real-time detection display at `http://localhost:5000` (optional)
- **Serial Terminal**: Live device output in the web interface or serial monitor
- **Audio Alerts**: Immediate notification of detections (device-side)
- **Heartbeat**: Continuous monitoring while devices in range
- **Range Tracking**: Automatic detection of device departure
- **Export Options**: Download detections as CSV or KML files (web interface)

### Channel Information
- **WiFi**: Automatically hops through channels 1-13
- **BLE**: Continuous scanning across all BLE channels
- **Status Updates**: Channel changes logged to serial terminal

## Detection Patterns

### SSID Patterns
- `flock*` - Flock Safety cameras
- `Penguin*` - Penguin surveillance devices
- `Pigvision*` - Pigvision systems
- `FS_*` - Flock Safety variants

### MAC Address Prefixes
- `AA:BB:CC` - Flock Safety manufacturer codes
- `DD:EE:FF` - Penguin device identifiers
- `11:22:33` - Pigvision system codes

### BLE Device Names
- `Flock*` - Flock Safety BLE devices
- `Penguin*` - Penguin BLE identifiers
- `Pigvision*` - Pigvision BLE devices

### Raven Service UUIDs (NEW)
- `0000180a-0000-1000-8000-00805f9b34fb` - Device Information Service
- `00003100-0000-1000-8000-00805f9b34fb` - GPS Location Service
- `00003200-0000-1000-8000-00805f9b34fb` - Power Management Service
- `00003300-0000-1000-8000-00805f9b34fb` - Network Status Service
- `00003400-0000-1000-8000-00805f9b34fb` - Upload Statistics Service
- `00003500-0000-1000-8000-00805f9b34fb` - Error/Failure Service
- `00001809-0000-1000-8000-00805f9b34fb` - Health Service (Legacy 1.1.x)
- `00001819-0000-1000-8000-00805f9b34fb` - Location Service (Legacy 1.1.x)

## Configuration System (ESP32-WROOM-32)

The system supports flexible configuration via `config.json` on the SD card. See [CONFIGURATION.md](CONFIGURATION.md) for complete details.

### Quick Configuration

1. **Copy example config to SD card**:
   ```bash
   cp config.json.example /path/to/sdcard/config.json
   ```

2. **Edit for your hardware**:
   ```json
   {
     "hardware": {
       "enable_gps": true,
       "enable_leds": true,
       "enable_buzzer": true,
       "buzzer_is_passive": false,
       "enable_oled": true,
       "enable_sd_card": true
     }
   }
   ```

3. **Hardware options**:
   - Set `enable_gps: false` if no GPS module
   - Set `enable_leds: false` if no LED strip
   - Set `enable_buzzer: false` if no buzzer
   - Set `buzzer_is_passive: true` for 3-pin PWM buzzer (false for 2-pin active)
   - Set `enable_oled: false` if no OLED display
   - Set `enable_sd_card: false` to disable database/logging

### Configuration Categories

- **Hardware**: Enable/disable components (GPS, LEDs, buzzer, OLED, SD card)
- **Scan**: Tuning (channel hop speed, BLE intervals, RSSI threshold)
- **Audio**: Beep durations and audio enable/disable
- **Display**: GPS/RSSI display, brightness, update rate
- **Log**: Verbose logging, flush interval, auto-export

### Effect of Disabling Hardware

- **GPS disabled**: Location shown as 0,0, no coordinate tracking
- **LEDs disabled**: No visual alerts (use buzzer/display/serial)
- **Buzzer disabled**: Silent operation
- **OLED disabled**: No local display (use serial monitor)
- **SD card disabled**: No database, exports, or persistent storage

See [CONFIGURATION.md](CONFIGURATION.md) for advanced configuration options.

## Database System (ESP32-WROOM-32)

The system maintains a persistent detection database on the SD card with in-memory caching for fast lookups.

### Database Files

- `/detections.db`: Device detections (MAC, Type, RSSI, FirstSeen, LastSeen, Count)
- `/locations.db`: GPS locations per device (MAC, lat1,lon1;lat2,lon2;...)
- `/device_index.idx`: Quick lookup index (MAC addresses)

### Database Features

- **In-Memory Cache**: HashMap with 500 device capacity (<1ms lookup)
- **Auto-Flush**: Writes to SD card every 30 seconds
- **Known Device Detection**: Different alerts for new vs re-detected devices
- **Location Tracking**: Multiple GPS coordinates per device
- **Export Function**: GeoJSON (OpenStreetMap) and CSV formats

### Exporting Data

1. **Hold BOOT button** (GPIO 0) for 2 seconds
2. **GeoJSON file**: `/export_YYYYMMDD_HHMMSS.geojson` (for OpenStreetMap)
3. **CSV file**: `/export_YYYYMMDD_HHMMSS.csv` (for spreadsheets)
4. **LED feedback**: Blue pulse during export

### Using Exported Data

**OpenStreetMap**:
1. Go to https://www.openstreetmap.org
2. Click "Export" → "Manually select a different area"
3. Use uMap or similar to import GeoJSON
4. Visualize detection locations on map

**Spreadsheet Analysis**:
1. Open CSV in Excel/Google Sheets
2. Analyze detection patterns, RSSI, timestamps
3. Create charts and reports

### Legacy Dataset Conversion

Convert existing CSV datasets to the new database format:

```powershell
.\convert-datasets.ps1
```

This converts all CSV files in the `datasets/` folder and imports them into the database.

See [SD_CARD_GUIDE.md](SD_CARD_GUIDE.md) for file structure details.

## Limitations

### Technical Constraints
- **WiFi Range**: Limited to 2.4GHz spectrum
- **Detection Range**: Approximately 50-100 meters depending on environment
- **False Positives**: Possible with similar device signatures
- **Power**: USB-powered operation (portable battery packs can be used)

### Environmental Factors
- **Interference**: Other WiFi networks may affect detection
- **Obstacles**: Walls and structures reduce detection range
- **Weather**: Outdoor conditions may impact performance

## Troubleshooting

### Common Issues - ESP32-WROOM-32 DevKit V4
1. **OLED Not Working**: 
   - Check I2C connections (SDA=GPIO21, SCL=GPIO22)
   - Verify I2C address is 0x3C
   - Disable in config.json if not installed: `"enable_oled": false`
2. **GPS Not Getting Fix**: 
   - Ensure GPS module has clear view of sky
   - Wait 1-5 minutes for initial fix (cold start)
   - Check UART connections (RX=GPIO16, TX=GPIO17)
   - Disable in config.json if not installed: `"enable_gps": false`
3. **SD Card Not Detected**:
   - Check SPI connections (CS=GPIO15, MOSI=GPIO13, MISO=GPIO12, SCK=GPIO14)
   - Ensure SD card is formatted as FAT32 (FAT16/exFAT also supported)
   - Try different SD card (some cards are incompatible)
   - Check serial output for filesystem detection errors
4. **LEDs Not Working**:
   - Check data connection to GPIO5
   - Ensure 5V power supply is adequate (4 LEDs need ~240mA max)
   - Verify LED strip type (NEO_GRB + NEO_KHZ800)
   - Disable in config.json if not installed: `"enable_leds": false`
5. **Buzzer Issues**:
   - **Active buzzer (2-pin)**: Set `"buzzer_is_passive": false`
   - **Passive buzzer (3-pin)**: Set `"buzzer_is_passive": true`
   - Check connection to GPIO23
   - Verify power (active=5V, passive=3.3V or 5V)
   - Disable in config.json if not installed: `"enable_buzzer": false`
6. **Database Not Loading**:
   - Check SD card is inserted and detected
   - Verify `/detections.db`, `/locations.db`, `/device_index.idx` exist
   - Check serial output for database errors
   - Database auto-creates on first detection if missing
7. **Export Not Working**:
   - Hold BOOT button (GPIO 0) for full 2 seconds
   - Check SD card has space (each export ~1-10KB per 100 devices)
   - Watch for blue LED pulse during export
   - Check serial output for export status
8. **Config.json Not Loading**:
   - Verify file is named exactly `config.json` (not `.txt`)
   - Check file is in SD card root directory
   - Validate JSON syntax (use https://jsonlint.com)
   - Check serial output: "Loading settings from config.json" should appear
9. **Yellow LED Alerts** (known devices always trigger):
   - This is normal - device is in database
   - Delete database files to reset detection history
   - Or wait - database tracks first/last seen times
10. **No Detections**:
    - Check scanning is active (green breathing LEDs)
    - Verify dual-core tasks started (check serial output)
    - Ensure RSSI threshold isn't too strict (default: -85dBm)
    - Increase BLE scan duration in config.json (try 10s)

### Common Issues - All Platforms
1. **Web Server Won't Start**: Check Python version (3.8+) and virtual environment setup
2. **No Serial Output**: Check USB connection and device port selection in web interface
3. **No Audio**: Verify buzzer connection to correct GPIO pin
4. **No Detections**: Ensure device is in range and scanning is active
5. **False Alerts**: Review detection patterns and adjust if needed
6. **Connection Issues**: Verify device is connected via the web interface controls

### Debug Information
- **Web Dashboard**: Real-time status and connection monitoring at `http://localhost:5000`
- **Serial Terminal**: Live device output in the web interface
- **Channel Hopping**: Logs channel changes for debugging
- **Detection Logs**: Full JSON output for analysis

## Legal and Ethical Considerations

### Intended Use
- **Research and Education**: Understanding surveillance technology
- **Security Assessment**: Evaluating privacy implications
- **Technical Analysis**: Studying wireless communication patterns

### Compliance
- **Local Laws**: Ensure compliance with local regulations
- **Privacy Rights**: Respect individual privacy and property rights
- **Authorized Use**: Only use in authorized locations and situations

## Credits and Research

### Research Foundation
This project is based on extensive research and public datasets from the surveillance detection community:

- **[DeFlock](https://deflock.me)** - Crowdsourced ALPR location and reporting tool
  - GitHub: [FoggedLens/deflock](https://github.com/FoggedLens/deflock)
  - Provides comprehensive datasets and methodologies for surveillance device detection
  - **Datasets included**: Real-world device signatures from deflock.me are included in the `datasets/` folder

- **[GainSec](https://github.com/GainSec)** - OSINT and privacy research
  - Specialized in surveillance technology analysis and detection methodologies
  - **Research referenced**: Some methodologies are based on their published research on surveillance technology
  - **Raven UUID Dataset Provider**: Contributed the `raven_configurations.json` dataset containing verified BLE service UUIDs from SoundThinking/ShotSpotter Raven devices across firmware versions 1.1.7, 1.2.0, and 1.3.1
  - Enables precise detection of Raven acoustic gunshot detection devices through BLE service UUID fingerprinting

### Methodology Integration
Flock You unifies multiple known detection methodologies into a comprehensive scanner/wardriver specifically designed for Flock Safety cameras and similar surveillance devices. The system combines:

- **WiFi Promiscuous Monitoring**: Based on DeFlock's network analysis techniques
- **BLE Device Detection**: Leveraging GainSec's Bluetooth surveillance research
- **MAC Address Filtering**: Using crowdsourced device databases from deflock.me
- **BLE Service UUID Fingerprinting**: Identifying Raven devices through advertised service characteristics
- **Firmware Version Detection**: Analyzing service combinations to determine device capabilities
- **Pattern Recognition**: Implementing research-based detection algorithms

### Acknowledgments
Special thanks to the researchers and contributors who have made this work possible through their open-source contributions and public datasets:

- **GainSec** for providing the comprehensive Raven BLE service UUID dataset, enabling detection of SoundThinking/ShotSpotter acoustic surveillance devices
- **DeFlock** for crowdsourced surveillance camera location data and detection methodologies
- The broader surveillance detection community for their continued research and privacy protection efforts

This project builds upon their foundational work in surveillance detection and privacy protection.



### Purchase Information
**Oui-Spy devices are available exclusively at [colonelpanic.tech](https://colonelpanic.tech)**

## License

This project is provided for educational and research purposes. Please ensure compliance with all applicable laws and regulations in your jurisdiction.

---

**Flock You: Professional surveillance detection for the privacy-conscious**
