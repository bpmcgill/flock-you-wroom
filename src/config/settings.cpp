#include "settings.h"
#include <SD.h>
#include <ArduinoJson.h>

SettingsManager settingsManager;

void SettingsManager::loadDefaults() {
    // Settings are already initialized with default values in struct definitions
    printf("Using default settings (all hardware enabled)\n");
}

bool SettingsManager::loadFromSD() {
    if (!SD.exists(CONFIG_FILE)) {
        printf("No config.json found, using defaults\n");
        loadDefaults();
        return false;
    }
    
    File file = SD.open(CONFIG_FILE, FILE_READ);
    if (!file) {
        printf("Failed to open config.json\n");
        loadDefaults();
        return false;
    }
    
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        printf("Failed to parse config.json: %s\n", error.c_str());
        loadDefaults();
        return false;
    }
    
    printf("Loading settings from config.json...\n");
    
    // Load hardware config
    JsonObject hw = doc["hardware"];
    if (!hw.isNull()) {
        settings.hardware.enable_gps = hw["enable_gps"] | true;
        settings.hardware.enable_leds = hw["enable_leds"] | true;
        settings.hardware.enable_buzzer = hw["enable_buzzer"] | true;
        settings.hardware.buzzer_is_passive = hw["buzzer_is_passive"] | false;
        settings.hardware.enable_oled = hw["enable_oled"] | true;
        settings.hardware.enable_sd_card = hw["enable_sd_card"] | true;
        
        // LED configuration
        settings.hardware.led_mode = (LEDMode)(hw["led_mode"] | 0);
        settings.hardware.led_brightness = hw["led_brightness"] | 50;
        settings.hardware.led0_function = (LEDFunction)(hw["led0_function"] | 1);
        settings.hardware.led1_function = (LEDFunction)(hw["led1_function"] | 2);
        settings.hardware.led2_function = (LEDFunction)(hw["led2_function"] | 3);
        settings.hardware.led3_function = (LEDFunction)(hw["led3_function"] | 4);
    }
    
    // Load scan config
    JsonObject scan = doc["scan"];
    if (!scan.isNull()) {
        settings.scan.channel_hop_interval = scan["channel_hop_interval"] | 200;
        settings.scan.ble_scan_duration = scan["ble_scan_duration"] | 5;
        settings.scan.ble_scan_interval = scan["ble_scan_interval"] | 100;
        settings.scan.rssi_threshold = scan["rssi_threshold"] | -85;
        settings.scan.detection_cooldown = scan["detection_cooldown"] | 2000;
    }
    
    // Load audio config
    JsonObject audio = doc["audio"];
    if (!audio.isNull()) {
        settings.audio.boot_beep_duration = audio["boot_beep_duration"] | 300;
        settings.audio.detect_beep_duration = audio["detect_beep_duration"] | 150;
        settings.audio.heartbeat_duration = audio["heartbeat_duration"] | 100;
        settings.audio.enable_audio = audio["enable_audio"] | true;
    }
    
    // Load display config
    JsonObject disp = doc["display"];
    if (!disp.isNull()) {
        settings.display.show_gps = disp["show_gps"] | true;
        settings.display.show_rssi = disp["show_rssi"] | true;
        settings.display.brightness = disp["brightness"] | 255;
        settings.display.update_interval = disp["update_interval"] | 1000;
    }
    
    // Load log config
    JsonObject log = doc["log"];
    if (!log.isNull()) {
        settings.log.verbose_logging = log["verbose_logging"] | false;
        settings.log.flush_interval = log["flush_interval"] | 30000;
        settings.log.auto_export = log["auto_export"] | false;
    }
    
    printf("Settings loaded successfully\n");
    return true;
}

bool SettingsManager::saveToSD() {
    StaticJsonDocument<1024> doc;
    
    // Hardware
    JsonObject hw = doc.createNestedObject("hardware");
    hw["enable_gps"] = settings.hardware.enable_gps;
    hw["enable_leds"] = settings.hardware.enable_leds;
    hw["enable_buzzer"] = settings.hardware.enable_buzzer;
    hw["buzzer_is_passive"] = settings.hardware.buzzer_is_passive;
    hw["enable_oled"] = settings.hardware.enable_oled;
    hw["enable_sd_card"] = settings.hardware.enable_sd_card;
    
    // LED configuration
    hw["led_mode"] = settings.hardware.led_mode;
    hw["led_brightness"] = settings.hardware.led_brightness;
    hw["led0_function"] = settings.hardware.led0_function;
    hw["led1_function"] = settings.hardware.led1_function;
    hw["led2_function"] = settings.hardware.led2_function;
    hw["led3_function"] = settings.hardware.led3_function;
    
    // Scan
    JsonObject scan = doc.createNestedObject("scan");
    scan["channel_hop_interval"] = settings.scan.channel_hop_interval;
    scan["ble_scan_duration"] = settings.scan.ble_scan_duration;
    scan["ble_scan_interval"] = settings.scan.ble_scan_interval;
    scan["rssi_threshold"] = settings.scan.rssi_threshold;
    scan["detection_cooldown"] = settings.scan.detection_cooldown;
    
    // Audio
    JsonObject audio = doc.createNestedObject("audio");
    audio["boot_beep_duration"] = settings.audio.boot_beep_duration;
    audio["detect_beep_duration"] = settings.audio.detect_beep_duration;
    audio["heartbeat_duration"] = settings.audio.heartbeat_duration;
    audio["enable_audio"] = settings.audio.enable_audio;
    
    // Display
    JsonObject disp = doc.createNestedObject("display");
    disp["show_gps"] = settings.display.show_gps;
    disp["show_rssi"] = settings.display.show_rssi;
    disp["brightness"] = settings.display.brightness;
    disp["update_interval"] = settings.display.update_interval;
    
    // Log
    JsonObject log = doc.createNestedObject("log");
    log["verbose_logging"] = settings.log.verbose_logging;
    log["flush_interval"] = settings.log.flush_interval;
    log["auto_export"] = settings.log.auto_export;
    
    File file = SD.open(CONFIG_FILE, FILE_WRITE);
    if (!file) {
        printf("Failed to create config.json\n");
        return false;
    }
    
    serializeJsonPretty(doc, file);
    file.close();
    
    printf("Settings saved to config.json\n");
    return true;
}

void SettingsManager::printSettings() {
    printf("\n=== Hardware Configuration ===\n");
    printf("GPS:     %s\n", settings.hardware.enable_gps ? "ENABLED" : "DISABLED");
    printf("LEDs:    %s\n", settings.hardware.enable_leds ? "ENABLED" : "DISABLED");
    printf("Buzzer:  %s\n", settings.hardware.enable_buzzer ? "ENABLED" : "DISABLED");
    printf("OLED:    %s\n", settings.hardware.enable_oled ? "ENABLED" : "DISABLED");
    printf("SD Card: %s\n", settings.hardware.enable_sd_card ? "ENABLED" : "DISABLED");
    
    printf("\n=== Scan Configuration ===\n");
    printf("WiFi Channel Hop: %d ms\n", settings.scan.channel_hop_interval);
    printf("BLE Scan Duration: %d s\n", settings.scan.ble_scan_duration);
    printf("BLE Scan Interval: %d ms\n", settings.scan.ble_scan_interval);
    printf("RSSI Threshold: %d dBm\n", settings.scan.rssi_threshold);
    
    printf("\n=== Audio Configuration ===\n");
    printf("Audio Enabled: %s\n", settings.audio.enable_audio ? "YES" : "NO");
    printf("Boot Beep: %d ms\n", settings.audio.boot_beep_duration);
    printf("Detect Beep: %d ms\n", settings.audio.detect_beep_duration);
    
    printf("\n==============================\n\n");
}
