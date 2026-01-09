# Complete Wiring Guide - Flock You Detection System

This document provides detailed wiring diagrams for all supported hardware configurations.

## Table of Contents
- [ESP32-WROOM-32 DevKit V4 (Full System)](#esp32-wroom-32-devkit-v4-full-system)
- [Pin Assignment Table](#pin-assignment-table)
- [Individual Component Wiring](#individual-component-wiring)
- [Xiao ESP32 S3 Basic Setup](#xiao-esp32-s3-basic-setup)
- [Power Requirements](#power-requirements)
- [Troubleshooting](#troubleshooting)

---

## ESP32-WROOM-32 DevKit V4 (Full System)

### Pin Assignment Table

| Component          | ESP32 GPIO | Pin Type | Notes                          |
|--------------------|------------|----------|--------------------------------|
| WS2812B LEDs       | GPIO 5     | Output   | Data line (4 LEDs)             |
| Buzzer             | GPIO 23    | Output   | 2-pin active OR 3-pin passive  |
| OLED SDA           | GPIO 21    | I2C      | I2C Data (SSD1315)             |
| OLED SCL           | GPIO 22    | I2C      | I2C Clock (SSD1315)            |
| RTC SDA            | GPIO 21    | I2C      | Shared with OLED (DS3231)      |
| RTC SCL            | GPIO 22    | I2C      | Shared with OLED (DS3231)      |
| GPS TX             | GPIO 16    | UART2 RX | GPS transmit → ESP32 receive   |
| GPS RX             | GPIO 17    | UART2 TX | GPS receive ← ESP32 transmit   |
| SD Card CS         | GPIO 15    | SPI      | Chip Select                    |
| SD Card MOSI       | GPIO 13    | SPI      | HSPI MOSI                      |
| SD Card MISO       | GPIO 12    | SPI      | HSPI MISO                      |
| SD Card SCK        | GPIO 14    | SPI      | HSPI Clock                     |
| BOOT Button        | GPIO 0     | Input    | Export database (hold 2s)      |

**Important Notes:**
- **I2C Bus Sharing:** OLED and RTC share the same I2C bus (GPIO 21/22). This is standard and supported.
- **I2C Addresses:** OLED (0x3C), RTC (0x68) - No conflicts
- **Boot Pin:** GPIO 0 has built-in pullup and BOOT button on most DevKit boards

---

## Individual Component Wiring

### 1. WS2812B LED Strip (4 LEDs)

**Purpose:** Visual feedback (scanning status, detection alerts, threat levels)

```
ESP32 DevKit V4           WS2812B LED Strip
┌─────────────┐          ┌──────────────┐
│   GPIO 5    │─────────>│ DIN (Data)   │
│   5V        │─────────>│ VCC (+5V)    │
│   GND       │─────────>│ GND          │
└─────────────┘          └──────────────┘
```

**Wiring Details:**
- **Data Pin (DIN):** Connect to GPIO 5
- **Power (VCC):** Connect to 5V pin on ESP32 DevKit
- **Ground (GND):** Connect to GND pin
- **Current Draw:** ~60mA per LED at full white brightness (240mA total for 4 LEDs)
- **Level Shifter:** Optional 3.3V→5V level shifter for data line (usually works without)

**Tips:**
- Add a 470Ω resistor between GPIO 5 and LED data pin to protect the first LED
- Add a 1000µF capacitor between VCC and GND on LED strip for stability
- Keep wires short to prevent signal degradation

---

### 2. SSD1315 OLED Display (128x64, I2C)

**Purpose:** Real-time status display (detection counts, GPS, signal strength)

```
ESP32 DevKit V4           SSD1315 OLED (128x64)
┌─────────────┐          ┌──────────────┐
│ GPIO21(SDA) │<────────>│ SDA          │
│ GPIO22(SCL) │─────────>│ SCL          │
│   3.3V      │─────────>│ VCC          │
│   GND       │─────────>│ GND          │
└─────────────┘          └──────────────┘
```

**Wiring Details:**
- **SDA (Data):** GPIO 21 (bi-directional)
- **SCL (Clock):** GPIO 22
- **Power:** 3.3V (some modules accept 5V, check specs)
- **I2C Address:** 0x3C (default for most SSD1306/SSD1315 displays)

**Tips:**
- Most OLED modules have built-in pull-up resistors
- If display doesn't work, try swapping SDA/SCL
- Use I2C scanner sketch to verify address if issues occur

---

### 3. DS3231 RTC Module (Real-Time Clock)

**Purpose:** Accurate timestamps independent of GPS

```
ESP32 DevKit V4           DS3231 RTC Module
┌─────────────┐          ┌──────────────┐
│ GPIO21(SDA) │<────────>│ SDA          │
│ GPIO22(SCL) │─────────>│ SCL          │
│   3.3V      │─────────>│ VCC          │
│   GND       │─────────>│ GND          │
└─────────────┘          └──────────────┘
```

**Wiring Details:**
- **SDA/SCL:** Shares I2C bus with OLED display
- **Power:** 3.3V recommended (module accepts 2.3V-5.5V)
- **Battery:** CR2032 coin cell for backup power
- **I2C Address:** 0x68 (fixed)

**Configuration:**
Set `enable_rtc: true` in config.json to enable RTC support.

**Features:**
- Auto-sync from GPS every hour
- Battery backup maintains time during power loss
- ±2ppm accuracy (~1 minute/year drift)
- Temperature sensor (±3°C accuracy)

**See:** [RTC_SETUP.md](RTC_SETUP.md) for complete setup guide.

---

### 4. GY-NEO6Mv2 GPS Module

**Purpose:** Location tracking for detections

```
ESP32 DevKit V4           GY-NEO6Mv2 GPS
┌─────────────┐          ┌──────────────┐
│ GPIO16(RX2) │<─────────│ TX           │
│ GPIO17(TX2) │─────────>│ RX           │
│ 3.3V or 5V  │─────────>│ VCC          │
│   GND       │─────────>│ GND          │
└─────────────┘          └──────────────┘
```

**Wiring Details:**
- **GPS TX → ESP32 RX:** GPIO 16 (UART2 receive)
- **GPS RX ← ESP32 TX:** GPIO 17 (UART2 transmit)
- **Power:** Most modules work with 3.3V or 5V (check your module)
- **Antenna:** Ceramic patch antenna (built-in on most modules)

**Tips:**
- GPS requires clear view of sky for satellite fix
- First fix can take 30-60 seconds (cold start)
- Warm start (after power cycle) typically 1-5 seconds
- Red LED on module blinks when searching, solid when fix acquired

---

### 5. MicroSD Card Module (SPI)

**Purpose:** Data logging, configuration storage, database

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

**Wiring Details:**
- **CS (Chip Select):** GPIO 15
- **MOSI (Data Out):** GPIO 13
- **MISO (Data In):** GPIO 12
- **SCK (Clock):** GPIO 14
- **Power:** 5V (module has onboard 3.3V regulator)

**Supported Cards:**
- MicroSD (up to 2GB, FAT16)
- MicroSDHC (4GB-32GB, FAT32)
- Format as FAT32 for best compatibility

**Files Created:**
- `/config.json` - System configuration
- `/detections.db` - Detection database
- `/locations.db` - GPS location records
- `/device_index.idx` - Device index
- `/export_data.csv` - Exported CSV data
- `/export_map.geojson` - Exported GeoJSON map

---

### 6. Active Buzzer (2-pin, 5V)

**Purpose:** Audio alerts for detections

```
ESP32 DevKit V4           Active Buzzer (5V)
┌─────────────┐          ┌──────────────┐
│  GPIO 23    │─────────>│ Positive (+) │
│   GND       │─────────>│ Negative (-) │
└─────────────┘          └──────────────┘
```

**Configuration:**
Set `buzzer_is_passive: false` in config.json.

**Wiring Details:**
- **Positive:** GPIO 23 (direct connection)
- **Negative:** GND
- **Current:** Typically 20-30mA
- **Voltage:** 5V (can work with 3.3V but quieter)

**Behavior:**
- Boot: 2 beeps
- Detection: 3 beeps
- Heartbeat: 2 beeps every 10 seconds

---

### 7. Passive Buzzer (3-pin, PWM)

**Purpose:** Musical tone alerts (frequency control)

```
ESP32 DevKit V4           Passive Buzzer (3-pin)
┌─────────────┐          ┌──────────────┐
│  GPIO 23    │─────────>│ I/O (Signal) │
│ 3.3V or 5V  │─────────>│ VCC          │
│   GND       │─────────>│ GND          │
└─────────────┘          └──────────────┘
```

**Configuration:**
Set `buzzer_is_passive: true` in config.json.

**Wiring Details:**
- **Signal (I/O):** GPIO 23 (PWM output)
- **Power (VCC):** 5V recommended for louder volume (3.3V works but quieter)
- **Ground (GND):** GND

**Frequencies Used:**
- Boot sequence: C4 (262Hz), E4 (330Hz), G4 (392Hz), C5 (523Hz)
- New device: 2500Hz (high pitch)
- Known device: 1500Hz (lower pitch)
- Heartbeat: 1800Hz

---

## Complete System Diagram

### Full Wiring Overview (All Components)

```
                              ┌─────────────────────────────────┐
                              │     ESP32 DevKit V4             │
                              │                                 │
                         ┌────│ 5V      GND        3.3V         │─────┐
                         │    │                                 │     │
            ┌────────────┼────│ GPIO Pins:                      │─────┼─────┐
            │            │    │  5, 12, 13, 14, 15, 16, 17,     │     │     │
            │            │    │  21, 22, 23                     │     │     │
            │            │    └─────────────────────────────────┘     │     │
            │            │                                            │     │
       ┌────▼──┐    ┌────▼────┐      ┌──────────┐      ┌──────────┐   │     │
       │ LEDs  │    │   GPS   │      │   OLED   │      │    SD    │   │     │
       │(GPIO5)│    │ (16,17) │      │ (21,22)  │      │ (12-15)  │   │     │
       └───────┘    └─────────┘      └─────┬────┘      └──────────┘   │     │
                                            │                         │     │
                                      ┌─────▼────┐                    │     │
                                      │   RTC    │              ┌─────▼───┐ │
                                      │ (21,22)  │              │ Buzzer  │ │
                                      │ (Shared) │              │ (GPIO23)│ │
                                      └──────────┘              └─────────┘ │
                                                                            │
                                                                            │
                                      Power Distribution ───────────────────┘
                                      
   I2C Bus (GPIO 21, 22):                      SPI Bus (GPIO 12-15):
   - OLED Display (0x3C)                       - SD Card Module
   - RTC Module (0x68)
   
   UART2 (GPIO 16, 17):                        PWM/Digital (GPIO 23):
   - GPS Module                                - Active/Passive Buzzer
   
   Digital Output (GPIO 5):                    Digital Input (GPIO 0):
   - WS2812B LED Strip                         - BOOT Button (built-in)
```

### I2C Bus Detail (Shared by OLED and RTC)

```
                    ESP32 DevKit V4
                    ┌──────────────┐
                    │ GPIO 21 (SDA)│<────┬─── OLED SDA (0x3C)
                    │ GPIO 22 (SCL)│─────┼─── OLED SCL
                    │              │     │
                    │              │     ├─── RTC SDA (0x68)
                    │              │     └─── RTC SCL
                    │              │
                    │ 3.3V         │───── OLED VCC, RTC VCC
                    │ GND          │───── OLED GND, RTC GND
                    └──────────────┘

Note: Both devices share the same I2C bus. No conflicts due to different addresses.
Pull-up resistors typically included on OLED module (4.7kΩ typical).
```

---

## Xiao ESP32 S3 Basic Setup

### Standard Xiao ESP32 S3 Wiring

**Minimal Configuration (Buzzer Only):**

```
Xiao ESP32 S3         3V Buzzer
┌─────────────┐      ┌──────────────┐
│ GPIO3 (D2)  │─────>│ Positive (+) │
│   GND       │─────>│ Negative (-) │
└─────────────┘      └──────────────┘
```

**Xiao ESP32 S3 Pin Labels:**
- D2 = GPIO 3 (Buzzer output)
- 3V3 = 3.3V power output
- GND = Ground

### Oui-Spy Device (Pre-built)

The Oui-Spy device available at [colonelpanic.tech](https://colonelpanic.tech) has all components pre-wired:
- Built-in buzzer system
- Optimized PCB layout
- USB-C programming interface
- No external wiring required

---

## Power Requirements

### Total Current Draw (ESP32-WROOM-32 Full System)

| Component          | Typical Current | Max Current | Notes                      |
|--------------------|-----------------|-------------|----------------------------|
| ESP32 (active)     | 160mA           | 240mA       | WiFi/BLE scanning          |
| WS2812B LEDs (4x)  | 20mA            | 240mA       | 60mA per LED at full white |
| OLED Display       | 20mA            | 30mA        | Depends on pixels lit      |
| GPS Module         | 40mA            | 80mA        | Higher during acquisition  |
| SD Card Module     | 50mA            | 200mA       | Peaks during writes        |
| RTC Module         | 0.1mA           | 1mA         | Very low power             |
| Active Buzzer      | 20mA            | 30mA        | When sounding              |
| **Total**          | **310mA**       | **821mA**   | Peak during detection      |

**Power Supply Recommendations:**
- **USB Power:** 500mA minimum (standard USB 2.0)
- **USB 3.0/Wall Adapter:** 1A+ recommended for reliable operation
- **Battery Power:** 18650 Li-ion (3.7V, 2000mAh+) with boost converter to 5V
- **Expected Runtime:** ~3-4 hours on 2000mAh battery (depends on detection frequency)

**Power Optimization Tips:**
- Reduce LED brightness in config.json (default: 50/255)
- Disable unused components (GPS, OLED, buzzer)
- Use power-efficient LED modes (Status or Custom mode)

---

## Troubleshooting

### Common Wiring Issues

#### 1. LEDs Not Working
**Symptoms:** No LED output or flickering
**Solutions:**
- Verify 5V power connection to LED strip
- Check GPIO 5 data connection
- Try adding 470Ω resistor on data line
- Ensure LEDs are powered before ESP32 boots
- Check LED strip direction (DIN = data input, DOUT = data output)

#### 2. OLED Display Blank
**Symptoms:** Display doesn't light up
**Solutions:**
- Verify I2C connections (SDA=21, SCL=22)
- Check power (3.3V, not 5V on most modules)
- Try swapping SDA/SCL if module labeling is incorrect
- Run I2C scanner to verify address (should be 0x3C or 0x3D)
- Check display initialization in code (SSD1306 vs SSD1315)

#### 3. RTC Not Detected
**Symptoms:** "ERROR: Could not find DS3231 RTC!" message
**Solutions:**
- Verify I2C connections match OLED (shared bus)
- Check that RTC module is powered (3.3V)
- Install CR2032 battery in RTC module
- Run I2C scanner to confirm 0x68 address
- Verify RTC module is DS3231 (not DS1307 or other)

#### 4. GPS No Fix
**Symptoms:** GPS stays in "SEARCHING" or "NO_GPS" state
**Solutions:**
- Position GPS with clear view of sky (near window)
- Wait 30-60 seconds for cold start acquisition
- Verify TX/RX connections (TX→RX, RX→TX, crossed)
- Check GPS LED (should blink during search, solid when locked)
- Verify correct UART pins (GPS TX→GPIO16, GPS RX→GPIO17)

#### 5. SD Card Not Detected
**Symptoms:** "SD card not found" message
**Solutions:**
- Format SD card as FAT32 (not exFAT)
- Use card 32GB or smaller (SDHC compatible)
- Check all SPI connections (CS=15, MOSI=13, MISO=12, SCK=14)
- Verify 5V power to SD module
- Try different SD card (some cards incompatible)
- Ensure card is fully inserted

#### 6. Buzzer Silent
**Symptoms:** No sound from buzzer
**Solutions:**
- Verify GPIO 23 connection
- Check buzzer polarity (+ to GPIO23, - to GND for active)
- Verify `enable_buzzer: true` in config.json
- For passive buzzer: set `buzzer_is_passive: true`
- For active buzzer: set `buzzer_is_passive: false`
- Try 5V power instead of 3.3V (louder)

#### 7. I2C Device Conflicts
**Symptoms:** OLED or RTC works individually but not together
**Solutions:**
- Both devices should coexist (different addresses)
- Run I2C scanner with both connected
- Verify OLED=0x3C, RTC=0x68 (no overlap)
- Check for short circuits on SDA/SCL lines
- Ensure proper pull-up resistors (usually on OLED module)

### I2C Scanner Tool

If you experience I2C issues, use this scanner sketch to detect devices:

```cpp
#include <Wire.h>

void setup() {
  Wire.begin(21, 22); // SDA, SCL
  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
}

void loop() {
  byte error, address;
  int devices = 0;

  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      devices++;
    }
  }
  
  if (devices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("Scan complete\n");
    
  delay(5000);
}
```

**Expected Results:**
- OLED: 0x3C or 0x3D
- RTC DS3231: 0x68

---

## Component Sources

### Where to Buy Components

**ESP32 DevKit V4:**
- AliExpress, Amazon, DigiKey, Mouser

**WS2812B LED Strip:**
- Search for "WS2812B 4 LED strip" or individual LEDs

**SSD1315/SSD1306 OLED:**
- 128x64 I2C OLED display (blue/white or blue/yellow)

**DS3231 RTC Module:**
- Look for "DS3231 RTC module" with battery holder

**GY-NEO6Mv2 GPS:**
- Standard GPS module with ceramic patch antenna

**MicroSD Card Module:**
- SPI MicroSD card adapter module

**Buzzer:**
- Active: "5V active buzzer" (2-pin)
- Passive: "5V passive buzzer" (3-pin)

### Pre-Built Option

**Oui-Spy Device:**
Available at [colonelpanic.tech](https://colonelpanic.tech)
- All components integrated
- Professional PCB design
- Pre-tested and calibrated
- USB-C interface
- Compact form factor

---

## Recommended Build Order

For best results, wire and test components in this order:

1. **ESP32 DevKit V4** - Flash test firmware, verify USB connection
2. **OLED Display** - Test I2C communication, verify address
3. **RTC Module** - Add to I2C bus, verify both devices work
4. **LED Strip** - Test visual feedback system
5. **Buzzer** - Test audio alerts (active or passive)
6. **SD Card Module** - Test SPI communication and file operations
7. **GPS Module** - Test UART communication and satellite acquisition

This incremental approach helps isolate issues and verify each component before adding complexity.

---

## Final Assembly Tips

1. **Use Breadboard First:** Prototype on breadboard before soldering
2. **Label Wires:** Use colored wires or labels for clarity
3. **Secure Connections:** Use dupont connectors or solder for reliability
4. **Cable Management:** Keep wires organized and away from antenna areas
5. **Enclosure Planning:** Consider case design before final assembly
6. **Test Frequently:** Verify each component as you add it
7. **Document Changes:** Note any pin modifications or substitutions

---

## Support

For additional help:
- See [QUICK_START.md](QUICK_START.md) for software setup
- See [RTC_SETUP.md](RTC_SETUP.md) for RTC-specific details
- See [CONFIGURATION.md](CONFIGURATION.md) for config.json settings
- Check GitHub Issues for common problems and solutions
