# Flock You: Flock Safety Detection System

<img src="flock.png" alt="Flock You" width="300px">

**Professional surveillance camera detection for the Oui-Spy device available at [colonelpanic.tech](https://colonelpanic.tech)**

## Overview

Flock You is an advanced detection system designed to identify Flock Safety surveillance cameras, Raven gunshot detectors, and similar surveillance devices using multiple detection methodologies. Built for the Xiao ESP32 S3 microcontroller, it provides real-time monitoring with audio alerts and comprehensive JSON output. The system now includes specialized BLE service UUID fingerprinting for detecting SoundThinking/ShotSpotter Raven acoustic surveillance devices.

## Features

### Multi-Method Detection
- **WiFi Promiscuous Mode**: Captures probe requests and beacon frames
- **Bluetooth Low Energy (BLE) Scanning**: Monitors BLE advertisements
- **MAC Address Filtering**: Detects devices by known MAC prefixes
- **SSID Pattern Matching**: Identifies networks by specific names
- **Device Name Pattern Matching**: Detects BLE devices by advertised names
- **BLE Service UUID Detection**: Identifies Raven gunshot detectors by service UUIDs (NEW)

### Enhanced Visual & Audio Feedback System (ESP32-WROOM-32)
- **LED Animations**: 4x WS2812B RGB LEDs with color-coded alerts
  - **Boot Sequence**: Blue LED fade-in animation
  - **Scanning Mode**: Slow green pulse (breathing effect)
  - **WiFi Detection**: Blue flash
  - **BLE Detection**: Purple flash
  - **Detection Alert**: Fast red flashing (3 times)
  - **Raven Detection**: Red + white strobe (critical threat)
  - **Heartbeat Pulse**: Orange pulse every 10 seconds while device in range
- **Active Buzzer Alerts**: Simple on/off beeping
  - **Boot Sequence**: 2 beeps on startup
  - **Detection Alert**: 3 fast beeps when device detected
  - **Heartbeat**: 2 beeps every 10 seconds while device in range
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

### SD Card Data Logging (ESP32-WROOM-32)
- **CSV Log Files**: Automatic logging to MicroSD card
- **Detection Records**: Timestamp, protocol, MAC, RSSI, GPS coordinates
- **Persistent Storage**: Data survives power cycles

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
Active Buzzer      | GPIO 23    | 2-pin 5V active buzzer
OLED SDA           | GPIO 21    | I2C Data (SSD1315)
OLED SCL           | GPIO 22    | I2C Clock (SSD1315)
GPS TX             | GPIO 16    | UART2 RX (GPS transmit)
GPS RX             | GPIO 17    | UART2 TX (GPS receive)
SD Card CS         | GPIO 15    | SPI Chip Select
SD Card MOSI       | GPIO 13    | SPI MOSI (HSPI)
SD Card MISO       | GPIO 12    | SPI MISO (HSPI)
SD Card SCK        | GPIO 14    | SPI Clock (HSPI)
```

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

**Active Buzzer Connection:**
```
ESP32 DevKit V4           Active Buzzer (5V)
┌─────────────┐          ┌──────────────┐
│  GPIO 23    │─────────>│ Positive (+) │
│   GND       │─────────>│ Negative (-) │
└─────────────┘          └──────────────┘
```

**Complete System Wiring Overview:**
```
                           ┌──────────────────────┐
                           │  ESP32 DevKit V4     │
                           │                      │
                      ┌────│ 5V    GND     3.3V  │────┐
                      │    │                      │    │
         ┌────────────┼────│ GPIO Pins:          │────┼─────┐
         │            │    │  5, 12-17, 21-23    │    │     │
         │            │    └──────────────────────┘    │     │
         │            │                                │     │
    ┌────▼──┐    ┌───▼────┐    ┌────────┐    ┌──────▼──┐  │
    │ LEDs  │    │  GPS   │    │  OLED  │    │   SD    │  │
    │(GPIO5)│    │(16,17) │    │(21,22) │    │(12-15)  │  │
    └───────┘    └────────┘    └────────┘    └─────────┘  │
                                                            │
                                                      ┌─────▼──┐
                                                      │ Buzzer │
                                                      │(GPIO23)│
                                                      └────────┘
```

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
- **WiFi**: 2.4GHz 802.11 b/g/n
- **Bluetooth**: BLE 4.2
- **I2C**: Hardware I2C for OLED
- **SPI**: Hardware HSPI for SD card
- **UART**: UART0 (USB serial), UART2 (GPS)
- **GPIO**: 34 pins available
- **Power**: 3.3V logic, 5V USB input

### Xiao ESP32 S3
- **Flash Memory**: 8MB
- **PSRAM**: 8MB OPI
- **WiFi**: 2.4GHz 802.11 b/g/n
- **Bluetooth**: BLE 5.0
- **USB**: Native USB CDC

### WiFi Capabilities
- **Frequency**: 2.4GHz only (13 channels)
- **Mode**: Promiscuous monitoring
- **Channel Hopping**: Automatic cycling every 2 seconds
- **Packet Types**: Probe requests (0x04) and beacons (0x08)

### BLE Capabilities
- **Framework**: NimBLE-Arduino
- **Scan Mode**: Active scanning
- **Interval**: 100ms scan intervals
- **Window**: 99ms scan windows

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
2. **Watch the OLED display** show boot screen
3. **Listen for boot beeps** and observe **blue LED fade-in** animation
4. **Wait for initialization**:
   - LED strip initializes (all LEDs briefly light up)
   - OLED displays "FLOCK DETECTOR v2.0"
   - GPS module starts acquiring satellites
   - SD card mounts and creates log file
5. **Check OLED status**:
   - Status should show "SCANNING"
   - GPS status will show "SEARCHING" then "FIX" when locked
   - SD card shows "OK" or "ERR"
6. **System ready** when display shows all systems initialized
7. **Green breathing LEDs** indicate active scanning mode

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
  - Red flash: Flock Safety detection alert
  - Red/white strobe: Raven gunshot detector (critical threat)
  - Orange pulse: Heartbeat (device still in range)
- **Audio Alerts**: Buzzer beeps for detections and heartbeat
- **SD Card Logging**: All detections logged to CSV file with GPS coordinates
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
   - Try scanning I2C bus: `Wire.begin(); Wire.beginTransmission(0x3C);`
2. **GPS Not Getting Fix**: 
   - Ensure GPS module has clear view of sky
   - Wait 1-5 minutes for initial fix (cold start)
   - Check UART connections (RX=GPIO16, TX=GPIO17)
3. **SD Card Not Detected**:
   - Check SPI connections (CS=GPIO15, MOSI=GPIO13, MISO=GPIO12, SCK=GPIO14)
   - Ensure SD card is formatted as FAT32
   - Try lower SPI speed: `SD_SCK_MHZ(1)` instead of `SD_SCK_MHZ(4)`
4. **LEDs Not Working**:
   - Check data connection to GPIO5
   - Ensure 5V power supply is adequate (4 LEDs need ~240mA max)
   - Verify LED strip type (NEO_GRB + NEO_KHZ800)
5. **Active Buzzer Always On/Off**:
   - Verify buzzer is 5V active type (not passive/PWM)
   - Check connection to GPIO23
   - Test with simple digitalWrite HIGH/LOW

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
