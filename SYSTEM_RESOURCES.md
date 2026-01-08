# FlockYouWroom System Resource Usage

## ESP32-WROOM-32 Specifications

### Hardware Capabilities
- **CPU**: Dual-core Xtensa LX6 @ 240 MHz
  - Core 0: BLE scanning task (dedicated)
  - Core 1: WiFi, main loop, peripherals
- **SRAM**: 520 KB total
  - ~200 KB available for application
  - Rest used by WiFi/BLE stack, FreeRTOS
- **Flash**: 4 MB
  - ~1.5 MB for firmware
  - Rest available for filesystem (SPIFFS/LittleFS)
- **PSRAM**: None on WROOM-32 variant

## Current Resource Usage (Estimated)

### Flash Memory (Program Storage)
```
Component              Size (KB)    % of 1.5MB
─────────────────────────────────────────────
Core Framework         ~400         27%
WiFi Stack             ~200         13%
BLE Stack (NimBLE)     ~150         10%
Adafruit Libraries     ~80           5%
GPS/TinyGPS++          ~15           1%
SD/FAT Library         ~50           3%
Application Code       ~100          7%
JSON Processing        ~30           2%
─────────────────────────────────────────────
TOTAL                  ~1025 KB     68%
AVAILABLE              ~475 KB      32%
```

### RAM Usage (Runtime Memory)

#### Static Allocation
```
Component                  Size (KB)    Notes
────────────────────────────────────────────────────────
WiFi/BLE Stack             ~100        ESP-IDF managed
FreeRTOS                   ~30         Task management
Adafruit_NeoPixel          ~0.1        4 LEDs × 3 bytes
Adafruit_SSD1306           ~1.0        128×64 frame buffer
TinyGPSPlus                ~0.5        GPS parser state
SD Buffer                  ~4.0        512-byte sector cache
Database Cache (HashMap)   ~20-40      500 devices × 40-80 bytes
Detection State            ~2.0        Tracking variables
Config/Settings            ~1.0        JSON config in RAM
String Buffers             ~5.0        Serial output, temp strings
────────────────────────────────────────────────────────
TOTAL STATIC               ~160-180 KB
```

#### Dynamic Allocation (Heap)
```
Operation                  Peak Usage   Notes
────────────────────────────────────────────────────────
BLE Scan Results           ~10-20 KB    Per scan (100 devices)
WiFi Packet Processing     ~2-5 KB      Promiscuous mode buffer
JSON Parsing (config)      ~2 KB        ArduinoJson document
GeoJSON Export             ~15-30 KB    Per 100 devices
CSV Export                 ~10-20 KB    Per 100 devices
Database Flush             ~5-10 KB     Write buffer
────────────────────────────────────────────────────────
TYPICAL HEAP FREE          ~40-60 KB    During normal operation
```

### Task Stack Sizes
```
Task                Stack Size    Priority    Core    Notes
─────────────────────────────────────────────────────────────
BLE Scanner         8 KB          1           0       Dedicated task
Main Loop           8 KB          1           1       Default Arduino
WiFi Event          4 KB          23          0       ESP-IDF managed
TCP/IP              4 KB          18          0       ESP-IDF managed
Idle (Core 0)       1 KB          0           0       FreeRTOS
Idle (Core 1)       1 KB          0           1       FreeRTOS
```

## Performance Metrics

### CPU Utilization (Typical)
```
Core 0:
  BLE Scanning        ~30-40%
  BLE Processing      ~10-15%
  WiFi Events         ~5-10%
  Idle                ~35-55%

Core 1:
  WiFi Promiscuous    ~20-30%
  Main Loop           ~10-15%
  Display Update      ~5%
  GPS Processing      ~5%
  LED/Buzzer          ~1-2%
  SD Card I/O         ~3-5%
  Idle                ~40-55%
```

### Detection Performance
```
Metric                    Value              Notes
────────────────────────────────────────────────────────────
BLE Scan Cycle            5.1 seconds        5s scan + 100ms gap
WiFi Channel Cycle        2.6 seconds        13 channels × 200ms
Database Lookup           < 1 ms             In-memory HashMap
New Device Add            < 5 ms             Cache only
Database Flush            50-200 ms          30s intervals
GeoJSON Export            200-500 ms         Per 100 devices
Detection Latency         < 3 seconds        Typical worst-case
```

### Power Consumption
```
Mode                    Current (mA)    Power (W)    Notes
─────────────────────────────────────────────────────────────
Scanning (no LEDs)      ~150-170        0.75-0.85    Base operation
Scanning + LEDs (green) ~180-200        0.90-1.00    4 LEDs @ 50%
Detection Alert (red)   ~200-220        1.00-1.10    LEDs @ 100%
SD Card Write           +20-40          +0.10-0.20   During flush
GPS Active              +30-50          +0.15-0.25   Acquiring fix
Buzzer Active           +10-20          +0.05-0.10   Beeping
─────────────────────────────────────────────────────────────
TYPICAL AVERAGE         ~180-200 mA     0.90-1.00 W  @ 5V
```

### Battery Life Estimates
```
Battery Capacity    Runtime (Scanning)    Runtime (With Detections)
─────────────────────────────────────────────────────────────────────
1000 mAh            ~5 hours              ~4.5 hours
2000 mAh            ~10 hours             ~9 hours
5000 mAh            ~25 hours             ~22 hours
10000 mAh           ~50 hours             ~45 hours

Notes:
- Assumes 5V USB power bank
- Detection runtime includes periodic LED/buzzer alerts
- GPS and SD card active
```

## Database Scalability

### In-Memory Cache (HashMap)
```
Devices     RAM Usage    Lookup Time    Notes
──────────────────────────────────────────────────────────
100         ~4-8 KB      < 0.5 ms       Light load
500         ~20-40 KB    < 1 ms         Default capacity
1000        ~40-80 KB    < 2 ms         Requires more RAM
2000        ~80-160 KB   < 3 ms         Near RAM limit
```

**Current Configuration**: 500 devices (optimal for 520KB SRAM)

### SD Card Storage
```
Records     Database Size    Export Size (GeoJSON)    Export Size (CSV)
───────────────────────────────────────────────────────────────────────────
100         ~15 KB           ~8 KB                    ~10 KB
1000        ~150 KB          ~80 KB                   ~100 KB
10000       ~1.5 MB          ~800 KB                  ~1 MB
100000      ~15 MB           ~8 MB                    ~10 MB

SD Card: 2-32 GB recommended (FAT32)
```

## Optimization Notes

### Memory Optimizations Applied
1. **F() Macro**: All constant strings stored in flash, not RAM
2. **HashMap**: O(1) lookup instead of linear search
3. **Streaming I/O**: SD card operations use buffered writes
4. **Stack Sizes**: Tuned for actual usage (8KB BLE task)
5. **Static Buffers**: Pre-allocated where possible

### CPU Optimizations Applied
1. **Dual-Core**: BLE on Core 0, WiFi on Core 1 (parallelism)
2. **vTaskDelay**: Prevents watchdog timeouts, yields CPU
3. **Buffered SD**: Batch writes every 30 seconds
4. **RSSI Filter**: Skip weak signals (-85 dBm threshold)
5. **Detection Cooldown**: Prevents spam (2s minimum gap)

### Power Optimizations Possible (Not Yet Implemented)
1. **WiFi Power Save**: Could reduce scanning power by 20-30%
2. **BLE Interval Tuning**: Longer gaps save power but reduce coverage
3. **LED Brightness**: Lower brightness (25 instead of 50) saves ~10-20mA
4. **GPS Smart Mode**: Power off GPS between fixes
5. **Sleep Modes**: Deep sleep when no activity (future feature)

## Resource Limits & Warnings

### When You'll Hit Limits

**RAM Limit (~200 KB available):**
- ❌ Database cache > 1000 devices (need ~80-160 KB)
- ❌ Large JSON exports in memory (> 100 KB)
- ❌ Excessive string allocations (memory leaks)

**Flash Limit (~475 KB available):**
- ❌ Large firmware features without optimization
- ❌ Multiple BLE/WiFi stacks simultaneously

**CPU Limit (240 MHz dual-core):**
- ❌ BLE scan duration > 10s (impacts WiFi coverage)
- ❌ WiFi hop interval < 100ms (too fast, misses packets)
- ❌ Complex cryptography or heavy math

### Monitoring Resources

Add to main.cpp for real-time monitoring:
```cpp
void printSystemStats() {
    printf("Free Heap: %d KB\n", ESP.getFreeHeap() / 1024);
    printf("Min Free Heap: %d KB\n", ESP.getMinFreeHeap() / 1024);
    printf("Heap Size: %d KB\n", ESP.getHeapSize() / 1024);
    printf("PSRAM: %d KB\n", ESP.getPsramSize() / 1024);
    printf("Flash Size: %d KB\n", ESP.getFlashChipSize() / 1024);
    printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
}

// Call every 10 seconds in loop()
if (millis() % 10000 == 0) {
    printSystemStats();
}
```

## Recommendations

### Current Configuration is Optimal
✅ 500 device cache (good balance)  
✅ 8 KB BLE stack (sufficient)  
✅ 30s flush interval (prevents SD wear)  
✅ Dual-core utilization (maximizes throughput)  

### Future Expansion Possibilities
- Increase database to 1000 devices if GPS/OLED disabled
- Add WiFi power save mode for battery operation
- Implement deep sleep for wardriving mode (scan, sleep, repeat)
- Add OTA (Over-The-Air) firmware updates

### Not Recommended
- ❌ Disable dual-core (reduces detection coverage)
- ❌ Cache > 1000 devices (RAM pressure)
- ❌ BLE scan > 10s (impacts WiFi scanning)
- ❌ WiFi hop < 100ms (packet loss)
