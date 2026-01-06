#ifndef PINS_H
#define PINS_H

// ============================================================================
// PIN DEFINITIONS FOR ESP32 DEVKIT V4
// ============================================================================

// LED Strip
#define LED_PIN         5       // WS2812B LED strip data pin
#define LED_COUNT       4       // Number of WS2812B LEDs

// Buzzer
#define BUZZER_PIN      23      // Active buzzer (2-pin 5V)

// I2C (OLED Display)
#define OLED_SDA        21      // I2C Data for SSD1315 OLED
#define OLED_SCL        22      // I2C Clock for SSD1315 OLED

// UART (GPS)
#define GPS_RX          16      // UART2 RX (GPS TX)
#define GPS_TX          17      // UART2 TX (GPS RX)

// SPI (SD Card)
#define SD_CS           15      // SD Card Chip Select
#define SD_MOSI         13      // SD Card MOSI (HSPI)
#define SD_MISO         12      // SD Card MISO (HSPI)
#define SD_SCK          14      // SD Card Clock (HSPI)

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

// OLED Display
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define OLED_ADDRESS    0x3C

// Audio Timing (for active buzzer)
#define BOOT_BEEP_DURATION      300
#define DETECT_BEEP_DURATION    150
#define HEARTBEAT_DURATION      100

// WiFi Configuration
#define MAX_CHANNEL             13
#define CHANNEL_HOP_INTERVAL    500     // milliseconds

// BLE Configuration
#define BLE_SCAN_DURATION       1       // Seconds
#define BLE_SCAN_INTERVAL       5000    // Milliseconds between scans

#endif // PINS_H
