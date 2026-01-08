# LED Modes Configuration Guide

## Overview

The FlockYouWroom system features 4 individually addressable WS2812B RGB LEDs that can operate in 6 different modes. Configure the mode in `config.json` on your SD card.

## Quick Configuration

Edit `/config.json` on SD card:
```json
{
  "hardware": {
    "led_mode": 1,           // 0-5 (see modes below)
    "led_brightness": 50,    // 0-255
    "led0_function": 1,      // Custom mode only (0-7)
    "led1_function": 2,
    "led2_function": 3,
    "led3_function": 4
  }
}
```

---

## Mode 0: Unified (Default)

**All 4 LEDs show the same color - Legacy v1.x behavior**

```
[🟢][🟢][🟢][🟢]  Scanning (green breathing)
[🔴][🔴][🔴][🔴]  New detection (red flash 3x)
[🟡][🟡][🟡][🟡]  Known device (yellow flash 1x)
[🟠][🟠][🟠][🟠]  Heartbeat (orange pulse)
```

**When to use:**
- Simple visual feedback
- Maximum brightness (all LEDs same color)
- Don't need detailed status

---

## Mode 1: Status Dashboard

**Each LED shows different system status**

```
LED 0 (Power):  [🟢] System OK  |  [🔴] Error
LED 1 (WiFi):   [🔵] WiFi detected  |  [⚫] No WiFi
LED 2 (BLE):    [🟣] BLE detected   |  [⚫] No BLE
LED 3 (GPS):    [🟡] GPS locked     |  [🟠] Searching  |  [⚫] Disabled
```

**Example scenarios:**
```
[🟢][⚫][🟣][🟡]  System OK, BLE detected, GPS locked
[🟢][🔵][🟣][🟠]  WiFi + BLE detected, GPS searching
[🔴][⚫][⚫][⚫]  System error (SD card failed, etc.)
```

**When to use:**
- Real-time system monitoring
- Troubleshooting hardware issues
- Understanding which subsystems are active

---

## Mode 2: Signal Strength

**Bar graph showing RSSI of detected signals**

```
-90 dBm (very weak):  [⚫][⚫][⚫][⚫]  0 bars
-75 dBm (weak):       [🔴][⚫][⚫][⚫]  1 bar (red)
-65 dBm (medium):     [🟠][🟠][⚫][⚫]  2 bars (orange)
-50 dBm (good):       [🟡][🟡][🟡][⚫]  3 bars (yellow)
-35 dBm (strong):     [🟢][🟢][🟢][🟢]  4 bars (green)
```

**When to use:**
- Directional finding (wardriving)
- Signal strength optimization
- Antenna alignment

---

## Mode 3: Detection Counter

**Progressive milestone indicators**

```
0 detections:     [⚫][⚫][⚫][⚫]
1+ detections:    [🟢][⚫][⚫][⚫]  First detection!
10+ detections:   [🟢][🔵][⚫][⚫]  10 milestone
50+ detections:   [🟢][🔵][🟠][⚫]  50 milestone
100+ detections:  [🟢][🔵][🟠][🔴]  100 milestone (all lit!)
```

**When to use:**
- Gamification (achievement unlocks)
- Long-term monitoring sessions
- Visual detection count at a glance

---

## Mode 4: Threat Level

**Visual threat assessment based on detected device count**

```
0 devices:        [🟢][🟢][⚫][⚫]  Safe (2 green LEDs)
1-5 devices:      [🟡][🟡][🟡][⚫]  Low threat (3 yellow)
6-10 devices:     [🟠][🟠][🟠][⚫]  Medium threat (3 orange)
11+ devices:      [🔴][🔴][🔴][🔴]  High threat (ALL RED!)
```

**When to use:**
- Rapid threat assessment
- Surveillance density monitoring
- High-surveillance area warnings

---

## Mode 5: Custom

**User-defined function for each LED**

### LED Functions (0-7)

```
0 = Off          LED disabled/dark
1 = Power        🟢 System OK  |  🔴 Error
2 = WiFi         🔵 WiFi detected  |  ⚫ Inactive
3 = BLE          🟣 BLE detected   |  ⚫ Inactive
4 = GPS          🟡 GPS locked     |  ⚫ Searching
5 = SD Card      🟢 SD OK          |  🔴 SD Error
6 = Scanning     🟢 Scanning mode  |  ⚫ Idle
7 = Detection    🔴 Device found   |  ⚫ None
```

### Example Custom Configurations

**Configuration 1: Detection Focus**
```json
"led0_function": 7,  // Detection indicator
"led1_function": 2,  // WiFi active
"led2_function": 3,  // BLE active
"led3_function": 4   // GPS status
```
Result: `[🔴][🔵][🟣][🟡]` = Detection found, WiFi+BLE active, GPS locked

**Configuration 2: System Health**
```json
"led0_function": 1,  // Power/system
"led1_function": 4,  // GPS
"led2_function": 5,  // SD card
"led3_function": 6   // Scanning status
```
Result: `[🟢][🟡][🟢][🟢]` = All systems OK, GPS locked, scanning

**Configuration 3: Minimalist**
```json
"led0_function": 7,  // Detection only
"led1_function": 0,  // Off
"led2_function": 0,  // Off
"led3_function": 0   // Off
```
Result: `[🔴][⚫][⚫][⚫]` = Single LED for detections, rest dark

---

## Mode Comparison

| Mode | Use Case | Info Density | Power | Complexity |
|------|----------|--------------|-------|------------|
| 0 - Unified | Simple alerts | Low | High | ⭐ |
| 1 - Status | System monitoring | High | Medium | ⭐⭐ |
| 2 - Signal | Wardriving | Medium | Low-Med | ⭐⭐ |
| 3 - Counter | Long sessions | Low | Low | ⭐ |
| 4 - Threat | Area assessment | Medium | Medium | ⭐⭐ |
| 5 - Custom | Specific needs | Variable | Variable | ⭐⭐⭐ |

---

## Brightness Settings

```json
"led_brightness": 50   // 0-255 (default: 50)
```

**Recommended values:**
- `25` - Low brightness (power saving, indoor)
- `50` - Medium brightness (default, balanced)
- `100` - High brightness (outdoor daylight)
- `255` - Maximum brightness (direct sunlight, high power)

**Power consumption:**
- 25 brightness: ~10-15 mA per LED
- 50 brightness: ~20-30 mA per LED
- 255 brightness: ~60 mA per LED

---

## Switching Modes On-The-Fly

1. **Edit config.json** on SD card (ESP32 powered off)
2. **Change `led_mode`** value (0-5)
3. **Re-insert SD card** and power on
4. **New mode active** immediately

No firmware reflashing required!

---

## Troubleshooting

### LEDs Not Working
- Verify `enable_leds: true` in config.json
- Check LED strip connection (GPIO 5)
- Ensure 5V power supply adequate
- Test with Mode 0 (simplest)

### Wrong Colors
- Check LED strip type (should be GRB, not RGB)
- Verify in pins.h: `NEO_GRB + NEO_KHZ800`

### Mode Not Changing
- Confirm config.json syntax is valid
- Check serial output for "LED Mode: X"
- Verify SD card detected

### Custom Mode Not Working
- Ensure `led_mode: 5`
- Check `led0_function` through `led3_function` values (0-7)
- Invalid function numbers default to OFF

---

## Advanced: Creating New Modes

Want to create your own LED mode? Edit `src/hardware/led_controller.cpp`:

```cpp
case LED_MODE_CUSTOM:
    // Your custom logic here
    LED.updateCustomMode(...);
    break;
```

See `led_controller.h` for available methods.

---

## Example Workflows

### Wardriving Session
```json
"led_mode": 2,           // Signal strength bars
"led_brightness": 100    // Bright for outdoor
```
Watch the bars grow as you approach surveillance!

### Stationary Monitoring
```json
"led_mode": 4,           // Threat level
"led_brightness": 50     // Medium indoor
```
Gradual color change as more devices appear.

### Stealth Operation
```json
"led_mode": 5,           // Custom
"led0_function": 7,      // Detection only
"led_brightness": 25     // Dim
```
Minimal visual signature, just detection alerts.

### System Debugging
```json
"led_mode": 1,           // Status dashboard
"led_brightness": 50     // Medium
```
See exactly which subsystems are active.

---

## LED Mode Summary

**Quick Reference:**
- `0` = All LEDs same (simple)
- `1` = Power/WiFi/BLE/GPS (status)
- `2` = RSSI bars (signal strength)
- `3` = 1/10/50/100 milestones (counter)
- `4` = Threat level (safety)
- `5` = Your choice (custom)

Configure in `/config.json` → `hardware.led_mode`
