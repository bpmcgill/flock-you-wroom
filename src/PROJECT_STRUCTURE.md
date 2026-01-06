# Flock You - Project Structure

## Overview
The project has been refactored into a modular architecture that separates concerns and makes development easier.

## Directory Structure

```
src/
├── main.cpp                    # Main application entry point (setup & loop)
├── config/                     # Configuration files
│   ├── pins.h                  # Hardware pin definitions
│   └── patterns.h              # Detection patterns (SSIDs, MACs, UUIDs)
├── hardware/                   # Hardware abstraction layer
│   ├── led_controller.h/cpp    # WS2812B LED strip control
│   ├── buzzer.h/cpp            # Active buzzer control
│   ├── display.h/cpp           # SSD1306 OLED display
│   ├── gps_manager.h/cpp       # GPS module interface
│   └── sd_logger.h/cpp         # SD card logging
└── detection/                  # Detection logic
    ├── detection_state.h/cpp   # Centralized detection state
    ├── wifi_detector.h/cpp     # WiFi promiscuous mode detection
    ├── ble_detector.h/cpp      # BLE scanning and detection
    └── raven_detector.h/cpp    # Raven-specific UUID detection
```

## Module Descriptions

### Configuration (`config/`)
- **pins.h**: All hardware pin definitions and configuration constants
- **patterns.h**: Detection patterns for Flock Safety and Raven devices

### Hardware Layer (`hardware/`)
Each hardware component has its own class with a clean interface:

- **LEDController**: Manages WS2812B LED strip with preset colors and effects
- **Buzzer**: Controls active buzzer for alerts and notifications
- **Display**: Manages OLED display with multiple screens
- **GPSManager**: GPS data acquisition and formatting
- **SDLogger**: CSV logging to SD card with automatic file management

### Detection Layer (`detection/`)
Modular detection system with clear separation:

- **DetectionState**: Centralized state management for all detections
- **WiFiDetector**: WiFi promiscuous mode packet sniffing
- **BLEDetector**: BLE advertisement scanning
- **RavenDetector**: Specialized Raven device detection via service UUIDs

## Benefits

### 1. **Separation of Concerns**
Each module has a single, well-defined responsibility

### 2. **Easy Testing**
Individual modules can be tested in isolation

### 3. **Maintainability**
Changes to one component don't affect others

### 4. **Reusability**
Hardware modules can be reused in other ESP32 projects

### 5. **Readability**
main.cpp is now < 100 lines instead of 1000+

## How to Add New Features

### Adding a new detection pattern:
Edit `src/config/patterns.h`

### Adding a new hardware module:
1. Create header/cpp in `src/hardware/`
2. Create singleton instance with `extern`
3. Include in `main.cpp` and call `.begin()` in `setup()`

### Modifying detection logic:
Edit the appropriate detector in `src/detection/`

## Global Instances

All hardware and detection modules are available as global singletons:
- `LED` - LED controller
- `buzzer` - Buzzer controller
- `display` - OLED display
- `gpsManager` - GPS module
- `sdLogger` - SD card logger
- `wifiDetector` - WiFi detector
- `bleDetector` - BLE detector
- `detectionState` - Detection state manager

## Building

The project structure is fully compatible with PlatformIO. Simply run:
```bash
pio run
```

All `.cpp` files in `src/` and subdirectories are automatically compiled.
