#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

// LED mode definitions
enum LEDMode {
    LED_MODE_UNIFIED = 0,      // All LEDs same color (default, v1.x behavior)
    LED_MODE_STATUS = 1,       // LED0=Power, LED1=WiFi, LED2=BLE, LED3=GPS
    LED_MODE_SIGNAL = 2,       // Signal strength bar graph (0-4 bars)
    LED_MODE_COUNTER = 3,      // Detection counter (1/10/50/100 milestones)
    LED_MODE_THREAT = 4,       // Threat level indicator (green/yellow/orange/red)
    LED_MODE_CUSTOM = 5        // User-defined LED assignments
};

// Custom LED function assignments
enum LEDFunction {
    LED_FUNC_OFF = 0,          // LED disabled/off
    LED_FUNC_POWER = 1,        // System power/status
    LED_FUNC_WIFI = 2,         // WiFi detection indicator
    LED_FUNC_BLE = 3,          // BLE detection indicator
    LED_FUNC_GPS = 4,          // GPS lock status
    LED_FUNC_SD = 5,           // SD card status
    LED_FUNC_SCANNING = 6,     // Scanning status
    LED_FUNC_DETECTION = 7     // Detection alert
};

// Hardware enable/disable flags
struct HardwareConfig {
    bool enable_gps = true;
    bool enable_rtc = false;                // DS3231 RTC for accurate timestamps
    bool enable_leds = true;
    bool enable_buzzer = true;
    bool buzzer_is_passive = false;         // true for 3-pin passive, false for 2-pin active
    bool enable_oled = true;
    bool enable_sd_card = true;
    
    // LED configuration
    LEDMode led_mode = LED_MODE_UNIFIED;    // LED behavior mode
    uint8_t led_brightness = 50;            // 0-255
    LEDFunction led0_function = LED_FUNC_POWER;   // Custom mode: LED 0 assignment
    LEDFunction led1_function = LED_FUNC_WIFI;    // Custom mode: LED 1 assignment
    LEDFunction led2_function = LED_FUNC_BLE;     // Custom mode: LED 2 assignment
    LEDFunction led3_function = LED_FUNC_GPS;     // Custom mode: LED 3 assignment
};

// Scanning and detection parameters
struct ScanConfig {
    uint16_t channel_hop_interval = 200;    // ms
    uint8_t ble_scan_duration = 5;          // seconds
    uint16_t ble_scan_interval = 100;       // ms
    int8_t rssi_threshold = -85;            // dBm
    uint16_t detection_cooldown = 2000;     // ms
};

// Audio feedback settings
struct AudioConfig {
    uint16_t boot_beep_duration = 300;      // ms
    uint16_t detect_beep_duration = 150;    // ms
    uint16_t heartbeat_duration = 100;      // ms
    bool enable_audio = true;
};

// Display settings
struct DisplayConfig {
    bool show_gps = true;
    bool show_rssi = true;
    uint8_t brightness = 255;
    uint16_t update_interval = 1000;        // ms
};

// Logging settings
struct LogConfig {
    bool verbose_logging = false;
    uint32_t flush_interval = 30000;        // ms
    bool auto_export = false;
};

// Complete system settings
struct SystemSettings {
    HardwareConfig hardware;
    ScanConfig scan;
    AudioConfig audio;
    DisplayConfig display;
    LogConfig log;
};

class SettingsManager {
public:
    void loadDefaults();
    bool loadFromSD();
    bool saveToSD();
    void printSettings();
    
    SystemSettings& getSettings() { return settings; }
    HardwareConfig& getHardware() { return settings.hardware; }
    
private:
    SystemSettings settings;
    const char* CONFIG_FILE = "/config.json";
};

extern SettingsManager settingsManager;

#endif // SETTINGS_H
