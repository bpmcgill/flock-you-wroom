# Configuration Guide

## Hardware Configuration via config.json

The FlockYouWroom system now supports flexible hardware configuration through a `config.json` file on the SD card. This allows you to enable/disable hardware components without recompiling firmware.

## Quick Setup

1. **Copy example config:**
   ```bash
   # Copy config.json.example to your SD card as config.json
   cp config.json.example /path/to/sdcard/config.json
   ```

2. **Edit for your hardware:**
   - Set `enable_gps: false` if no GPS module
   - Set `enable_leds: false` if no LED strip
   - Set `enable_buzzer: false` if no buzzer
   - Set `enable_oled: false` if no OLED display
   - Set `enable_sd_card: false` to disable database (detection still works)

3. **Insert SD card and power on**
   - Device reads config on boot
   - Only enabled hardware is initialized
   - System adapts to missing components

## Configuration File Reference

### Hardware Section

```json
"hardware": {
  "enable_gps": true,      // GPS module (NEO-6M/7M/8M)
  "enable_leds": true,     // WS2812B LED strip
  "enable_buzzer": true,   // Active or passive buzzer
  "buzzer_is_passive": false,  // true = 3-pin passive (PWM tones), false = 2-pin active
  "enable_oled": true,     // SSD1306 OLED display
  "enable_sd_card": true,  // SD card logging/database
  
  // LED Configuration
  "led_mode": 0,           // 0=Unified, 1=Status, 2=Signal, 3=Counter, 4=Threat, 5=Custom
  "led_brightness": 50,    // 0-255 (default: 50)
  "led0_function": 1,      // Custom mode: LED 0 function (0-7)
  "led1_function": 2,      // Custom mode: LED 1 function
  "led2_function": 3,      // Custom mode: LED 2 function
  "led3_function": 4       // Custom mode: LED 3 function
}
```

**Effect of disabling:**
- `GPS`: No location tracking, coordinates shown as 0,0
- `LEDs`: No visual alerts (detection still logged to serial)
- `Buzzer`: No audio alerts (detection still logged to serial)
- `OLED`: No display output (use serial monitor instead)
- `SD Card`: No database or export (runtime detection only)

**LED Modes:**
- `0` **Unified** (Default): All 4 LEDs same color (legacy v1.x behavior)
  - Green breathing = Scanning
  - Red flash = New detection
  - Yellow flash = Known device
  
- `1` **Status Dashboard**: Each LED shows different system status
  - LED 0 (Power): Green=OK, Red=Error
  - LED 1 (WiFi): Blue when WiFi device detected
  - LED 2 (BLE): Purple when BLE device detected
  - LED 3 (GPS): Yellow=Locked, Orange=Searching, Off=Disabled
  
- `2` **Signal Strength**: Bar graph showing RSSI
  - 0 bars = Very weak (-90 dBm)
  - 1 bar = Weak (red)
  - 2 bars = Medium (orange)
  - 3 bars = Good (yellow)
  - 4 bars = Strong (green, -30 dBm)
  
- `3` **Detection Counter**: Progressive milestone indicators
  - LED 0: Lights up after 1st detection (green)
  - LED 1: Lights up after 10 detections (blue)
  - LED 2: Lights up after 50 detections (orange)
  - LED 3: Lights up after 100 detections (red)
  
- `4` **Threat Level**: Visual threat assessment
  - 0 devices: LED 0-1 green (safe)
  - 1-5 devices: LED 0-2 yellow (low threat)
  - 6-10 devices: LED 0-2 orange (medium threat)
  - 11+ devices: LED 0-3 red (high threat)
  
- `5` **Custom**: User-defined LED assignments
  - Assign each LED a specific function (0-7)
  - Functions: 0=Off, 1=Power, 2=WiFi, 3=BLE, 4=GPS, 5=SD, 6=Scanning, 7=Detection

**LED Custom Functions** (used when `led_mode: 5`):\n- `0` **Off**: LED disabled\n- `1` **Power**: Green=System OK, Red=Error\n- `2` **WiFi**: Blue when WiFi detection active\n- `3` **BLE**: Purple when BLE detection active\n- `4` **GPS**: Yellow=GPS locked, Off=Searching\n- `5` **SD**: Green=SD OK, Red=SD Error\n- `6` **Scanning**: Green when scanning mode\n- `7` **Detection**: Red when device detected

**Buzzer types:**
- `buzzer_is_passive: false` - 2-pin active buzzer (default)
  - Simple beeps (on/off)
  - Louder at 3.3V
  - Wiring: VCC → GPIO23, GND → GND
  
- `buzzer_is_passive: true` - 3-pin passive buzzer
  - Musical tones (PWM controlled)
  - Quieter at 3.3V (use 5V for louder)
  - Different sounds: 1500Hz (known device), 2500Hz (new device)
  - Boot sequence: C4-E4-G4-C5 musical scale
  - Wiring: I/O → GPIO23, VCC → 3.3V or 5V, GND → GND

### Scan Section

```json
"scan": {
  "channel_hop_interval": 200,   // WiFi channel hop speed (ms)
  "ble_scan_duration": 5,        // BLE scan duration (seconds)
  "ble_scan_interval": 100,      // Gap between BLE scans (ms)
  "rssi_threshold": -85,         // Minimum signal strength (dBm)
  "detection_cooldown": 2000     // Cooldown between alerts (ms)
}
```

**Performance tuning:**
- **Faster scanning:** Lower `channel_hop_interval` (100-200ms)
- **Better BLE coverage:** Higher `ble_scan_duration` (5-10s)
- **More sensitivity:** Higher `rssi_threshold` (-90 to -70)

### Audio Section

```json
"audio": {
  "boot_beep_duration": 300,     // Boot beep length (ms)
  "detect_beep_duration": 150,   // Detection beep length (ms)
  "heartbeat_duration": 100,     // Heartbeat beep length (ms)
  "enable_audio": true           // Master audio enable
}
```

**Silent mode:** Set `enable_audio: false` for no beeps

### Display Section

```json
"display": {
  "show_gps": true,          // Show GPS status on OLED
  "show_rssi": true,         // Show signal strength
  "brightness": 255,         // LED brightness (0-255)
  "update_interval": 1000    // Display refresh rate (ms)
}
```

### Log Section

```json
"log": {
  "verbose_logging": false,  // Extra debug output
  "flush_interval": 30000,   // Database write interval (ms)
  "auto_export": false       // Auto-export on shutdown
}
```

## Hardware Configuration Examples

### Minimal Setup (WiFi/BLE only, no peripherals)

```json
{
  "hardware": {
    "enable_gps": false,
    "enable_leds": false,
    "enable_buzzer": false,
    "enable_oled": false,
    "enable_sd_card": false
  }
}
```

**Use case:** Headless detection with serial output only

### Car Setup (No display, audio alerts only)

```json
{
  "hardware": {
    "enable_gps": true,       // Track locations
    "enable_leds": true,      // Visual alerts
    "enable_buzzer": true,    // Audio alerts
    "enable_oled": false,     // No room for display
    "enable_sd_card": true    // Log everything
  }
}
```

### Stealth Mode (Silent, no lights)

```json
{
  "hardware": {
    "enable_gps": true,
    "enable_leds": false,     // No lights
    "enable_buzzer": false,   // Silent
    "enable_oled": true,      // Display only
    "enable_sd_card": true
  },
  "audio": {
    "enable_audio": false     // Extra silent
  }
}
```

### Development/Testing (All serial output)

```json
{
  "hardware": {
    "enable_gps": false,
    "enable_leds": false,
    "enable_buzzer": false,
    "enable_oled": false,
    "enable_sd_card": false
  },
  "log": {
    "verbose_logging": true   // Maximum debug info
  }
}
```

## Runtime Behavior

### Without GPS
- Location shown as `0.0, 0.0` on display
- Detections logged without coordinates
- Export map will show all points at origin
- System continues normal operation

### Without LEDs
- No visual feedback
- Detection alerts shown in serial monitor
- Use buzzer or display for feedback

### Without Buzzer
- Silent operation
- Detection alerts shown on LEDs/display/serial

### Without OLED
- No local display
- Use serial monitor (115200 baud) for status
- All detection info still available

### Without SD Card
- No persistent storage
- Detections logged to serial only
- No export functionality
- Database disabled (always "new" devices)

## Changing Configuration

### Method 1: Edit on Computer
1. Power off ESP32
2. Remove SD card
3. Edit `config.json` on computer
4. Re-insert SD card
5. Power on (new config loaded)

### Method 2: Save from Serial
```cpp
// In serial monitor, type 'S' to save current settings
// (Future feature - not yet implemented)
```

## Troubleshooting

### Config Not Loading
**Symptoms:** Default settings used, "No config.json found"

**Solutions:**
- Verify file is named exactly `config.json` (not `.txt`)
- Check file is in SD card root (not in folder)
- Verify JSON syntax (use online validator)

### Hardware Still Initializing When Disabled
**Symptoms:** GPS initializes even with `enable_gps: false`

**Solutions:**
- Check JSON syntax (missing comma, bracket)
- Verify `config.json` loads successfully (check serial)
- Re-save config.json with proper formatting

### Performance Issues
**Symptoms:** Slow scanning, missed detections

**Solutions:**
- Increase `channel_hop_interval` (200-400ms)
- Decrease `ble_scan_duration` (3-5s)
- Lower `rssi_threshold` (-85 to -80)
- Disable unused hardware

## Configuration Priority

1. **config.json** (if exists on SD card)
2. **Default values** (if no SD card or no config file)
3. **Code defaults** (hardcoded in settings.h)

## Best Practices

1. **Always keep a backup** of working config.json
2. **Test changes incrementally** (change one setting at a time)
3. **Monitor serial output** when changing config
4. **Start with defaults** and tune from there
5. **Document your changes** in comments (not in JSON, use separate file)

## Valid JSON Syntax

✅ **Correct:**
```json
{
  "hardware": {
    "enable_gps": true,    // Comma after each item
    "enable_leds": false   // No comma on last item
  }
}
```

❌ **Incorrect:**
```json
{
  "hardware": {
    "enable_gps": true
    "enable_leds": false,  // Missing comma above, extra comma here
  },                        // Extra comma
}
```

## Getting Help

If config.json isn't working:

1. **Check serial output** (115200 baud)
   - Look for "Failed to parse config.json"
   - Error message shows what's wrong

2. **Validate JSON**
   - Use https://jsonlint.com
   - Paste your config.json
   - Fix any syntax errors

3. **Start from example**
   - Copy `config.json.example`
   - Make minimal changes
   - Test each change

4. **Delete and rebuild**
   - Remove config.json
   - Device uses defaults
   - Create new config from working state
