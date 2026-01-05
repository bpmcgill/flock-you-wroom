#include <Arduino.h>
#include <WiFi.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include <ArduinoJson.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>
#include <SPI.h>
#include <SdFat.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// Pin Definitions for ESP32 DevKit V4
#define LED_PIN 5          // WS2812B LED strip data pin
#define LED_COUNT 4        // Number of WS2812B LEDs
#define BUZZER_PIN 23      // Active buzzer (2-pin 5V)
#define OLED_SDA 21        // I2C Data for SSD1315 OLED
#define OLED_SCL 22        // I2C Clock for SSD1315 OLED
#define GPS_RX 16          // UART2 RX (GPS TX)
#define GPS_TX 17          // UART2 TX (GPS RX)
#define SD_CS 15           // SD Card Chip Select
#define SD_MOSI 13         // SD Card MOSI (HSPI)
#define SD_MISO 12         // SD Card MISO (HSPI)
#define SD_SCK 14          // SD Card Clock (HSPI)
#define BATTERY_PIN 34     // ADC for battery voltage monitoring

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

// Audio Configuration (for active buzzer - simple on/off)
#define BOOT_BEEP_DURATION 300
#define DETECT_BEEP_DURATION 150
#define HEARTBEAT_DURATION 100

// WiFi Promiscuous Mode Configuration
#define MAX_CHANNEL 13
#define CHANNEL_HOP_INTERVAL 500  // milliseconds

// BLE SCANNING CONFIGURATION
#define BLE_SCAN_DURATION 1    // Seconds
#define BLE_SCAN_INTERVAL 5000 // Milliseconds between scans
static unsigned long last_ble_scan = 0;

// Detection Pattern Limits
#define MAX_SSID_PATTERNS 10
#define MAX_MAC_PATTERNS 50
#define MAX_DEVICE_NAMES 20

// ============================================================================
// DETECTION PATTERNS (Extracted from Real Flock Safety Device Databases)
// ============================================================================

// WiFi SSID patterns to detect (case-insensitive)
static const char* wifi_ssid_patterns[] = {
    "flock",        // Standard Flock Safety naming
    "Flock",        // Capitalized variant
    "FLOCK",        // All caps variant
    "FS Ext Battery", // Flock Safety Extended Battery devices
    "Penguin",      // Penguin surveillance devices
    "Pigvision"     // Pigvision surveillance systems
};

// Known Flock Safety MAC address prefixes (from real device databases)
static const char* mac_prefixes[] = {
    // FS Ext Battery devices
    "58:8e:81", "cc:cc:cc", "ec:1b:bd", "90:35:ea", "04:0d:84", 
    "f0:82:c0", "1c:34:f1", "38:5b:44", "94:34:69", "b4:e3:f9",
    
    // Flock WiFi devices
    "70:c9:4e", "3c:91:80", "d8:f3:bc", "80:30:49", "14:5a:fc",
    "74:4c:a1", "08:3a:88", "9c:2f:9d", "94:08:53", "e4:aa:ea"
    
    // Penguin devices - these are NOT OUI based, so use local ouis
    // from the wigle.net db relative to your location 
    // "cc:09:24", "ed:c7:63", "e8:ce:56", "ea:0c:ea", "d8:8f:14",
    // "f9:d9:c0", "f1:32:f9", "f6:a0:76", "e4:1c:9e", "e7:f2:43",
    // "e2:71:33", "da:91:a9", "e1:0e:15", "c8:ae:87", "f4:ed:b2",
    // "d8:bf:b5", "ee:8f:3c", "d7:2b:21", "ea:5a:98"
};

// Device name patterns for BLE advertisement detection
static const char* device_name_patterns[] = {
    "FS Ext Battery",  // Flock Safety Extended Battery
    "Penguin",         // Penguin surveillance devices
    "Flock",           // Standard Flock Safety devices
    "Pigvision"        // Pigvision surveillance systems
};

// ============================================================================
// RAVEN SURVEILLANCE DEVICE UUID PATTERNS
// ============================================================================
// These UUIDs are specific to Raven surveillance devices (acoustic gunshot detection)
// Source: raven_configurations.json - firmware versions 1.1.7, 1.2.0, 1.3.1

// Raven Device Information Service (used across all firmware versions)
#define RAVEN_DEVICE_INFO_SERVICE       "0000180a-0000-1000-8000-00805f9b34fb"

// Raven GPS Location Service (firmware 1.2.0+)
#define RAVEN_GPS_SERVICE               "00003100-0000-1000-8000-00805f9b34fb"

// Raven Power/Battery Service (firmware 1.2.0+)
#define RAVEN_POWER_SERVICE             "00003200-0000-1000-8000-00805f9b34fb"

// Raven Network Status Service (firmware 1.2.0+)
#define RAVEN_NETWORK_SERVICE           "00003300-0000-1000-8000-00805f9b34fb"

// Raven Upload Statistics Service (firmware 1.2.0+)
#define RAVEN_UPLOAD_SERVICE            "00003400-0000-1000-8000-00805f9b34fb"

// Raven Error/Failure Service (firmware 1.2.0+)
#define RAVEN_ERROR_SERVICE             "00003500-0000-1000-8000-00805f9b34fb"

// Health Thermometer Service (firmware 1.1.7)
#define RAVEN_OLD_HEALTH_SERVICE        "00001809-0000-1000-8000-00805f9b34fb"

// Location and Navigation Service (firmware 1.1.7)
#define RAVEN_OLD_LOCATION_SERVICE      "00001819-0000-1000-8000-00805f9b34fb"

// Known Raven service UUIDs for detection
static const char* raven_service_uuids[] = {
    RAVEN_DEVICE_INFO_SERVICE,    // Device info (all versions)
    RAVEN_GPS_SERVICE,            // GPS data (1.2.0+)
    RAVEN_POWER_SERVICE,          // Battery/Solar (1.2.0+)
    RAVEN_NETWORK_SERVICE,        // LTE/WiFi status (1.2.0+)
    RAVEN_UPLOAD_SERVICE,         // Upload stats (1.2.0+)
    RAVEN_ERROR_SERVICE,          // Error tracking (1.2.0+)
    RAVEN_OLD_HEALTH_SERVICE,     // Old health service (1.1.7)
    RAVEN_OLD_LOCATION_SERVICE    // Old location service (1.1.7)
};

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static uint8_t current_channel = 1;
static unsigned long last_channel_hop = 0;
static bool triggered = false;
static bool device_in_range = false;
static unsigned long last_detection_time = 0;
static unsigned long last_heartbeat = 0;
static NimBLEScan* pBLEScan;

// Hardware Objects
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // UART2
SdFat sd;
SdFile logFile;

// Detection counters
static int wifi_detection_count = 0;
static int ble_detection_count = 0;
static int total_detection_count = 0;
static unsigned long last_display_update = 0;



// ============================================================================
// LED SYSTEM
// ============================================================================

// LED Color definitions
#define COLOR_OFF strip.Color(0, 0, 0)
#define COLOR_BLUE strip.Color(0, 0, 255)
#define COLOR_GREEN strip.Color(0, 255, 0)
#define COLOR_RED strip.Color(255, 0, 0)
#define COLOR_ORANGE strip.Color(255, 165, 0)
#define COLOR_PURPLE strip.Color(128, 0, 128)
#define COLOR_WHITE strip.Color(255, 255, 255)

void setAllLEDs(uint32_t color) {
    for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, color);
    }
    strip.show();
}

void ledFadeIn(uint32_t color, int duration) {
    for (int brightness = 0; brightness <= 50; brightness += 5) {
        strip.setBrightness(brightness);
        setAllLEDs(color);
        delay(duration / 10);
    }
    strip.setBrightness(50);
}

void ledFlash(uint32_t color, int count, int duration) {
    for (int i = 0; i < count; i++) {
        setAllLEDs(color);
        delay(duration);
        setAllLEDs(COLOR_OFF);
        if (i < count - 1) delay(duration);
    }
}

void ledPulse(uint32_t color, int duration) {
    // Breathing effect
    for (int brightness = 10; brightness <= 50; brightness += 5) {
        strip.setBrightness(brightness);
        setAllLEDs(color);
        delay(duration / 16);
    }
    for (int brightness = 50; brightness >= 10; brightness -= 5) {
        strip.setBrightness(brightness);
        setAllLEDs(color);
        delay(duration / 16);
    }
    strip.setBrightness(50);
    setAllLEDs(COLOR_OFF);
}

void scanningLEDEffect() {
    static unsigned long lastPulse = 0;
    static bool pulsing = false;
    
    if (!pulsing && millis() - lastPulse > 3000) {
        pulsing = true;
        ledPulse(COLOR_GREEN, 500);
        pulsing = false;
        lastPulse = millis();
    }
}

void ravenDetectionStrobe() {
    // Red and white strobe for critical threat
    for (int i = 0; i < 5; i++) {
        setAllLEDs(COLOR_RED);
        delay(100);
        setAllLEDs(COLOR_WHITE);
        delay(100);
    }
    setAllLEDs(COLOR_OFF);
}

// ============================================================================
// ACTIVE BUZZER SYSTEM
// ============================================================================

void activeBuzzerBeep(int count, int duration, int gap) {
    for (int i = 0; i < count; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(duration);
        digitalWrite(BUZZER_PIN, LOW);
        if (i < count - 1) delay(gap);
    }
}

void boot_beep_sequence() {
    printf("Initializing audio system...\n");
    printf("Playing boot sequence\n");
    
    // Blue LED fade-in animation
    ledFadeIn(COLOR_BLUE, 500);
    delay(200);
    setAllLEDs(COLOR_OFF);
    
    // Two beeps
    activeBuzzerBeep(2, BOOT_BEEP_DURATION, 100);
    
    printf("Audio and LED system ready\n\n");
}

void flock_detected_beep_sequence() {
    printf("FLOCK SAFETY DEVICE DETECTED!\n");
    printf("Playing alert sequence: 3 fast beeps + LED flash\n");
    
    // Red LED flash
    ledFlash(COLOR_RED, 3, DETECT_BEEP_DURATION);
    
    // Three fast beeps
    activeBuzzerBeep(3, DETECT_BEEP_DURATION, 50);
    
    printf("Detection complete - device identified!\n\n");
    
    // Mark device as in range and start heartbeat tracking
    device_in_range = true;
    last_detection_time = millis();
    last_heartbeat = millis();
}

void heartbeat_pulse() {
    printf("Heartbeat: Device still in range\n");
    
    // Orange LED pulse
    ledPulse(COLOR_ORANGE, 400);
    
    // Two beeps
    activeBuzzerBeep(2, HEARTBEAT_DURATION, 100);
}

// ============================================================================
// BATTERY MONITORING
// ============================================================================

float getBatteryVoltage() {
    int raw = analogRead(BATTERY_PIN);
    // Assuming voltage divider: (raw / 4095.0) * 3.3V * 2
    return (raw / 4095.0) * 3.3 * 2.0;
}

int getBatteryPercentage() {
    float voltage = getBatteryVoltage();
    // 18650 battery: 4.2V full, 3.0V empty
    int percentage = map((int)(voltage * 10), 30, 42, 0, 100);
    return constrain(percentage, 0, 100);
}

// ============================================================================
// GPS SYSTEM
// ============================================================================

void updateGPS() {
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }
}

String getGPSLocation() {
    if (gps.location.isValid()) {
        return String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
    }
    return "NO_FIX";
}

String getGPSStatus() {
    if (gps.location.isValid()) {
        return "FIX";
    } else if (gps.satellites.value() > 0) {
        return "SEARCHING";
    }
    return "NO_GPS";
}

// ============================================================================
// SD CARD LOGGING
// ============================================================================

bool sd_initialized = false;

void initSDCard() {
    printf("Initializing SD card...\n");
    
    if (!sd.begin(SD_CS, SD_SCK_MHZ(4))) {
        printf("SD card initialization failed!\n");
        sd_initialized = false;
        return;
    }
    
    printf("SD card initialized successfully\n");
    sd_initialized = true;
    
    // Create log file
    char filename[32];
    sprintf(filename, "flock_%lu.csv", millis());
    
    if (logFile.open(filename, O_WRONLY | O_CREAT | O_TRUNC)) {
        // Write CSV header
        logFile.println("timestamp,protocol,detection_method,mac_address,rssi,ssid,device_name,gps_lat,gps_lon,battery_pct");
        logFile.close();
        printf("Created log file: %s\n", filename);
    } else {
        printf("Failed to create log file\n");
    }
}

void logDetectionToSD(const char* protocol, const char* method, const char* mac, 
                      int rssi, const char* ssid, const char* name) {
    if (!sd_initialized) return;
    
    char filename[32];
    sprintf(filename, "flock_%lu.csv", millis() / 1000000); // Group by million ms chunks
    
    if (logFile.open(filename, O_WRONLY | O_CREAT | O_APPEND)) {
        logFile.print(millis());
        logFile.print(",");
        logFile.print(protocol);
        logFile.print(",");
        logFile.print(method);
        logFile.print(",");
        logFile.print(mac);
        logFile.print(",");
        logFile.print(rssi);
        logFile.print(",");
        logFile.print(ssid ? ssid : "");
        logFile.print(",");
        logFile.print(name ? name : "");
        logFile.print(",");
        if (gps.location.isValid()) {
            logFile.print(gps.location.lat(), 6);
            logFile.print(",");
            logFile.print(gps.location.lng(), 6);
        } else {
            logFile.print(",");
        }
        logFile.print(",");
        logFile.println(getBatteryPercentage());
        logFile.close();
    }
}

// ============================================================================
// OLED DISPLAY SYSTEM
// ============================================================================

void displayBootScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("FLOCK DETECTOR v2.0"));
    display.println(F(""));
    display.println(F("ESP32-WROOM-32"));
    display.println(F("DevKit V4"));
    display.println(F(""));
    display.println(F("Initializing..."));
    display.display();
}

void updateDisplay() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Header
    display.setCursor(0, 0);
    display.println(F("FLOCK DETECTOR v2.0"));
    
    // Status
    display.print(F("Status: "));
    if (device_in_range) {
        display.println(F("DETECTED!"));
    } else {
        display.println(F("SCANNING"));
    }
    
    // Detection counts
    display.print(F("Detections: "));
    display.println(total_detection_count);
    display.print(F("WiFi: "));
    display.print(wifi_detection_count);
    display.print(F("  BLE: "));
    display.println(ble_detection_count);
    
    // GPS status
    display.print(F("GPS: "));
    if (gps.location.isValid()) {
        display.print(gps.location.lat(), 2);
        display.print(F(","));
        display.println(gps.location.lng(), 2);
    } else {
        display.println(getGPSStatus());
    }
    
    // Battery and SD status
    display.print(F("Batt: "));
    display.print(getBatteryPercentage());
    display.print(F("%  SD: "));
    display.println(sd_initialized ? F("OK") : F("ERR"));
    
    display.display();
}

void displayDetection(const char* deviceType, int rssi) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("ALERT!"));
    
    display.setTextSize(1);
    display.println(F(""));
    display.print(F("Type: "));
    display.println(deviceType);
    display.print(F("RSSI: "));
    display.print(rssi);
    display.println(F(" dBm"));
    
    if (gps.location.isValid()) {
        display.print(F("GPS: "));
        display.print(gps.location.lat(), 2);
        display.print(F(","));
        display.println(gps.location.lng(), 2);
    }
    
    display.display();
    delay(2000); // Show alert for 2 seconds
}

// ============================================================================
// JSON OUTPUT FUNCTIONS
// ============================================================================

void output_wifi_detection_json(const char* ssid, const uint8_t* mac, int rssi, const char* detection_type)
{
    DynamicJsonDocument doc(2048);
    
    // Core detection info
    doc["timestamp"] = millis();
    doc["detection_time"] = String(millis() / 1000.0, 3) + "s";
    doc["protocol"] = "wifi";
    doc["detection_method"] = detection_type;
    doc["alert_level"] = "HIGH";
    doc["device_category"] = "FLOCK_SAFETY";
    
    // WiFi specific info
    doc["ssid"] = ssid;
    doc["ssid_length"] = strlen(ssid);
    doc["rssi"] = rssi;
    doc["signal_strength"] = rssi > -50 ? "STRONG" : (rssi > -70 ? "MEDIUM" : "WEAK");
    doc["channel"] = current_channel;
    
    // MAC address info
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    doc["mac_address"] = mac_str;
    
    char mac_prefix[9];
    snprintf(mac_prefix, sizeof(mac_prefix), "%02x:%02x:%02x", mac[0], mac[1], mac[2]);
    doc["mac_prefix"] = mac_prefix;
    doc["vendor_oui"] = mac_prefix;
    
    // GPS data
    if (gps.location.isValid()) {
        doc["gps_latitude"] = gps.location.lat();
        doc["gps_longitude"] = gps.location.lng();
        doc["gps_altitude"] = gps.altitude.meters();
        doc["gps_satellites"] = gps.satellites.value();
    } else {
        doc["gps_status"] = getGPSStatus();
    }
    
    // Battery data
    doc["battery_percentage"] = getBatteryPercentage();
    doc["battery_voltage"] = getBatteryVoltage();
    
    // Detection pattern matching
    bool ssid_match = false;
    bool mac_match = false;
    
    for (int i = 0; i < sizeof(wifi_ssid_patterns)/sizeof(wifi_ssid_patterns[0]); i++) {
        if (strcasestr(ssid, wifi_ssid_patterns[i])) {
            doc["matched_ssid_pattern"] = wifi_ssid_patterns[i];
            doc["ssid_match_confidence"] = "HIGH";
            ssid_match = true;
            break;
        }
    }
    
    for (int i = 0; i < sizeof(mac_prefixes)/sizeof(mac_prefixes[0]); i++) {
        if (strncasecmp(mac_prefix, mac_prefixes[i], 8) == 0) {
            doc["matched_mac_pattern"] = mac_prefixes[i];
            doc["mac_match_confidence"] = "HIGH";
            mac_match = true;
            break;
        }
    }
    
    // Detection summary
    doc["detection_criteria"] = ssid_match && mac_match ? "SSID_AND_MAC" : (ssid_match ? "SSID_ONLY" : "MAC_ONLY");
    doc["threat_score"] = ssid_match && mac_match ? 100 : (ssid_match || mac_match ? 85 : 70);
    
    // Frame type details
    if (strcmp(detection_type, "probe_request") == 0 || strcmp(detection_type, "probe_request_mac") == 0) {
        doc["frame_type"] = "PROBE_REQUEST";
        doc["frame_description"] = "Device actively scanning for networks";
    } else {
        doc["frame_type"] = "BEACON";
        doc["frame_description"] = "Device advertising its network";
    }
    
    String json_output;
    serializeJson(doc, json_output);
    Serial.println(json_output);
    
    // Log to SD card
    logDetectionToSD("wifi", detection_type, mac_str, rssi, ssid, nullptr);
    
    // Update counters
    wifi_detection_count++;
    total_detection_count++;
    
    // Show LED color for WiFi detection
    ledFlash(COLOR_BLUE, 1, 200);
}

void output_ble_detection_json(const char* mac, const char* name, int rssi, const char* detection_method)
{
    DynamicJsonDocument doc(2048);
    
    // Core detection info
    doc["timestamp"] = millis();
    doc["detection_time"] = String(millis() / 1000.0, 3) + "s";
    doc["protocol"] = "bluetooth_le";
    doc["detection_method"] = detection_method;
    doc["alert_level"] = "HIGH";
    doc["device_category"] = "FLOCK_SAFETY";
    
    // BLE specific info
    doc["mac_address"] = mac;
    doc["rssi"] = rssi;
    doc["signal_strength"] = rssi > -50 ? "STRONG" : (rssi > -70 ? "MEDIUM" : "WEAK");
    
    // Device name info
    if (name && strlen(name) > 0) {
        doc["device_name"] = name;
        doc["device_name_length"] = strlen(name);
        doc["has_device_name"] = true;
    } else {
        doc["device_name"] = "";
        doc["device_name_length"] = 0;
        doc["has_device_name"] = false;
    }
    
    // MAC address analysis
    char mac_prefix[9];
    strncpy(mac_prefix, mac, 8);
    mac_prefix[8] = '\0';
    doc["mac_prefix"] = mac_prefix;
    doc["vendor_oui"] = mac_prefix;
    
    // GPS data
    if (gps.location.isValid()) {
        doc["gps_latitude"] = gps.location.lat();
        doc["gps_longitude"] = gps.location.lng();
        doc["gps_altitude"] = gps.altitude.meters();
        doc["gps_satellites"] = gps.satellites.value();
    } else {
        doc["gps_status"] = getGPSStatus();
    }
    
    // Battery data
    doc["battery_percentage"] = getBatteryPercentage();
    doc["battery_voltage"] = getBatteryVoltage();
    
    // Detection pattern matching
    bool name_match = false;
    bool mac_match = false;
    
    // Check MAC prefix patterns
    for (int i = 0; i < sizeof(mac_prefixes)/sizeof(mac_prefixes[0]); i++) {
        if (strncasecmp(mac, mac_prefixes[i], strlen(mac_prefixes[i])) == 0) {
            doc["matched_mac_pattern"] = mac_prefixes[i];
            doc["mac_match_confidence"] = "HIGH";
            mac_match = true;
            break;
        }
    }
    
    // Check device name patterns
    if (name && strlen(name) > 0) {
        for (int i = 0; i < sizeof(device_name_patterns)/sizeof(device_name_patterns[0]); i++) {
            if (strcasestr(name, device_name_patterns[i])) {
                doc["matched_name_pattern"] = device_name_patterns[i];
                doc["name_match_confidence"] = "HIGH";
                name_match = true;
                break;
            }
        }
    }
    
    // Detection summary
    doc["detection_criteria"] = name_match && mac_match ? "NAME_AND_MAC" : 
                               (name_match ? "NAME_ONLY" : "MAC_ONLY");
    doc["threat_score"] = name_match && mac_match ? 100 : 
                         (name_match || mac_match ? 85 : 70);
    
    // BLE advertisement type analysis
    doc["advertisement_type"] = "BLE_ADVERTISEMENT";
    doc["advertisement_description"] = "Bluetooth Low Energy device advertisement";
    
    // Detection method details
    if (strcmp(detection_method, "mac_prefix") == 0) {
        doc["primary_indicator"] = "MAC_ADDRESS";
        doc["detection_reason"] = "MAC address matches known Flock Safety prefix";
    } else if (strcmp(detection_method, "device_name") == 0) {
        doc["primary_indicator"] = "DEVICE_NAME";
        doc["detection_reason"] = "Device name matches Flock Safety pattern";
    }
    
    String json_output;
    serializeJson(doc, json_output);
    Serial.println(json_output);
    
    // Log to SD card
    logDetectionToSD("ble", detection_method, mac, rssi, nullptr, name);
    
    // Update counters
    ble_detection_count++;
    total_detection_count++;
    
    // Show LED color for BLE detection
    ledFlash(COLOR_PURPLE, 1, 200);
}

// ============================================================================
// DETECTION HELPER FUNCTIONS
// ============================================================================

bool check_mac_prefix(const uint8_t* mac)
{
    char mac_str[9];  // Only need first 3 octets for prefix check
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x", mac[0], mac[1], mac[2]);
    
    for (int i = 0; i < sizeof(mac_prefixes)/sizeof(mac_prefixes[0]); i++) {
        if (strncasecmp(mac_str, mac_prefixes[i], 8) == 0) {
            return true;
        }
    }
    return false;
}

bool check_ssid_pattern(const char* ssid)
{
    if (!ssid) return false;
    
    for (int i = 0; i < sizeof(wifi_ssid_patterns)/sizeof(wifi_ssid_patterns[0]); i++) {
        if (strcasestr(ssid, wifi_ssid_patterns[i])) {
            return true;
        }
    }
    return false;
}

bool check_device_name_pattern(const char* name)
{
    if (!name) return false;
    
    for (int i = 0; i < sizeof(device_name_patterns)/sizeof(device_name_patterns[0]); i++) {
        if (strcasestr(name, device_name_patterns[i])) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// RAVEN UUID DETECTION
// ============================================================================

// Check if a BLE device advertises any Raven surveillance service UUIDs
bool check_raven_service_uuid(NimBLEAdvertisedDevice* device, char* detected_service_out = nullptr)
{
    if (!device) return false;
    
    // Check if device has service UUIDs
    if (!device->haveServiceUUID()) return false;
    
    // Get the number of service UUIDs
    int serviceCount = device->getServiceUUIDCount();
    if (serviceCount == 0) return false;
    
    // Check each advertised service UUID against known Raven UUIDs
    for (int i = 0; i < serviceCount; i++) {
        NimBLEUUID serviceUUID = device->getServiceUUID(i);
        std::string uuidStr = serviceUUID.toString();
        
        // Compare against each known Raven service UUID
        for (int j = 0; j < sizeof(raven_service_uuids)/sizeof(raven_service_uuids[0]); j++) {
            if (strcasecmp(uuidStr.c_str(), raven_service_uuids[j]) == 0) {
                // Match found! Store the detected service UUID if requested
                if (detected_service_out != nullptr) {
                    strncpy(detected_service_out, uuidStr.c_str(), 40);
                }
                return true;
            }
        }
    }
    
    return false;
}

// Get a human-readable description of the Raven service
const char* get_raven_service_description(const char* uuid)
{
    if (!uuid) return "Unknown Service";
    
    if (strcasecmp(uuid, RAVEN_DEVICE_INFO_SERVICE) == 0)
        return "Device Information (Serial, Model, Firmware)";
    if (strcasecmp(uuid, RAVEN_GPS_SERVICE) == 0)
        return "GPS Location Service (Lat/Lon/Alt)";
    if (strcasecmp(uuid, RAVEN_POWER_SERVICE) == 0)
        return "Power Management (Battery/Solar)";
    if (strcasecmp(uuid, RAVEN_NETWORK_SERVICE) == 0)
        return "Network Status (LTE/WiFi)";
    if (strcasecmp(uuid, RAVEN_UPLOAD_SERVICE) == 0)
        return "Upload Statistics Service";
    if (strcasecmp(uuid, RAVEN_ERROR_SERVICE) == 0)
        return "Error/Failure Tracking Service";
    if (strcasecmp(uuid, RAVEN_OLD_HEALTH_SERVICE) == 0)
        return "Health/Temperature Service (Legacy)";
    if (strcasecmp(uuid, RAVEN_OLD_LOCATION_SERVICE) == 0)
        return "Location Service (Legacy)";
    
    return "Unknown Raven Service";
}

// Estimate firmware version based on detected service UUIDs
const char* estimate_raven_firmware_version(NimBLEAdvertisedDevice* device)
{
    if (!device || !device->haveServiceUUID()) return "Unknown";
    
    bool has_new_gps = false;
    bool has_old_location = false;
    bool has_power_service = false;
    
    int serviceCount = device->getServiceUUIDCount();
    for (int i = 0; i < serviceCount; i++) {
        NimBLEUUID serviceUUID = device->getServiceUUID(i);
        std::string uuidStr = serviceUUID.toString();
        
        if (strcasecmp(uuidStr.c_str(), RAVEN_GPS_SERVICE) == 0)
            has_new_gps = true;
        if (strcasecmp(uuidStr.c_str(), RAVEN_OLD_LOCATION_SERVICE) == 0)
            has_old_location = true;
        if (strcasecmp(uuidStr.c_str(), RAVEN_POWER_SERVICE) == 0)
            has_power_service = true;
    }
    
    // Firmware version heuristics based on service presence
    if (has_old_location && !has_new_gps)
        return "1.1.x (Legacy)";
    if (has_new_gps && !has_power_service)
        return "1.2.x";
    if (has_new_gps && has_power_service)
        return "1.3.x (Latest)";
    
    return "Unknown Version";
}

// ============================================================================
// WIFI PROMISCUOUS MODE HANDLER
// ============================================================================

typedef struct {
    unsigned frame_ctrl:16;
    unsigned duration_id:16;
    uint8_t addr1[6]; /* receiver address */
    uint8_t addr2[6]; /* sender address */
    uint8_t addr3[6]; /* filtering address */
    unsigned sequence_ctrl:16;
    uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type)
{
    
    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
    const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;
    
    // Check for probe requests (subtype 0x04) and beacons (subtype 0x08)
    uint8_t frame_type = (hdr->frame_ctrl & 0xFF) >> 2;
    if (frame_type != 0x20 && frame_type != 0x80) { // Probe request or beacon
        return;
    }
    
    // Extract SSID from probe request or beacon
    char ssid[33] = {0};
    uint8_t *payload = (uint8_t *)ipkt + 24; // Skip MAC header
    
    if (frame_type == 0x20) { // Probe request
        payload += 0; // Probe requests start with SSID immediately
    } else { // Beacon frame
        payload += 12; // Skip fixed parameters in beacon
    }
    
    // Parse SSID element (tag 0, length, data)
    if (payload[0] == 0 && payload[1] <= 32) {
        memcpy(ssid, &payload[2], payload[1]);
        ssid[payload[1]] = '\0';
    }
    
    // Check if SSID matches our patterns
    if (strlen(ssid) > 0 && check_ssid_pattern(ssid)) {
        const char* detection_type = (frame_type == 0x20) ? "probe_request" : "beacon";
        output_wifi_detection_json(ssid, hdr->addr2, ppkt->rx_ctrl.rssi, detection_type);
        
        if (!triggered) {
            triggered = true;
            flock_detected_beep_sequence();
        }
        // Always update detection time for heartbeat tracking
        last_detection_time = millis();
        return;
    }
    
    // Check MAC address
    if (check_mac_prefix(hdr->addr2)) {
        const char* detection_type = (frame_type == 0x20) ? "probe_request_mac" : "beacon_mac";
        output_wifi_detection_json(ssid[0] ? ssid : "hidden", hdr->addr2, ppkt->rx_ctrl.rssi, detection_type);
        
        if (!triggered) {
            triggered = true;
            flock_detected_beep_sequence();
        }
        // Always update detection time for heartbeat tracking
        last_detection_time = millis();
        return;
    }
}

// ============================================================================
// BLE SCANNING
// ============================================================================

class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        
        NimBLEAddress addr = advertisedDevice->getAddress();
        std::string addrStr = addr.toString();
        uint8_t mac[6];
        sscanf(addrStr.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", 
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        
        int rssi = advertisedDevice->getRSSI();
        std::string name = "";
        if (advertisedDevice->haveName()) {
            name = advertisedDevice->getName();
        }
        
        // Check MAC prefix
        if (check_mac_prefix(mac)) {
            output_ble_detection_json(addrStr.c_str(), name.c_str(), rssi, "mac_prefix");
            if (!triggered) {
                triggered = true;
                flock_detected_beep_sequence();
            }
            // Always update detection time for heartbeat tracking
            last_detection_time = millis();
            return;
        }
        
        // Check device name
        if (!name.empty() && check_device_name_pattern(name.c_str())) {
            output_ble_detection_json(addrStr.c_str(), name.c_str(), rssi, "device_name");
            if (!triggered) {
                triggered = true;
                flock_detected_beep_sequence();
            }
            // Always update detection time for heartbeat tracking
            last_detection_time = millis();
            return;
        }
        
        // Check for Raven surveillance device service UUIDs
        char detected_service_uuid[41] = {0};
        if (check_raven_service_uuid(advertisedDevice, detected_service_uuid)) {
            // Raven device detected! Get firmware version estimate
            const char* fw_version = estimate_raven_firmware_version(advertisedDevice);
            const char* service_desc = get_raven_service_description(detected_service_uuid);
            
            // Create enhanced JSON output with Raven-specific data
            DynamicJsonDocument doc(2048);
            doc["timestamp"] = millis();
            doc["detection_time"] = String(millis() / 1000.0, 3) + "s";
            doc["protocol"] = "bluetooth_le";
            doc["detection_method"] = "raven_service_uuid";
            doc["device_type"] = "RAVEN_GUNSHOT_DETECTOR";
            doc["manufacturer"] = "SoundThinking/ShotSpotter";
            doc["mac_address"] = addrStr.c_str();
            doc["rssi"] = rssi;
            doc["signal_strength"] = rssi > -50 ? "STRONG" : (rssi > -70 ? "MEDIUM" : "WEAK");
            
            if (!name.empty()) {
                doc["device_name"] = name.c_str();
            }
            
            // GPS data
            if (gps.location.isValid()) {
                doc["gps_latitude"] = gps.location.lat();
                doc["gps_longitude"] = gps.location.lng();
                doc["gps_altitude"] = gps.altitude.meters();
                doc["gps_satellites"] = gps.satellites.value();
            } else {
                doc["gps_status"] = getGPSStatus();
            }
            
            // Battery data
            doc["battery_percentage"] = getBatteryPercentage();
            doc["battery_voltage"] = getBatteryVoltage();
            
            // Raven-specific information
            doc["raven_service_uuid"] = detected_service_uuid;
            doc["raven_service_description"] = service_desc;
            doc["raven_firmware_version"] = fw_version;
            doc["threat_level"] = "CRITICAL";
            doc["threat_score"] = 100;
            
            // List all detected service UUIDs
            if (advertisedDevice->haveServiceUUID()) {
                JsonArray services = doc.createNestedArray("service_uuids");
                int serviceCount = advertisedDevice->getServiceUUIDCount();
                for (int i = 0; i < serviceCount; i++) {
                    NimBLEUUID serviceUUID = advertisedDevice->getServiceUUID(i);
                    services.add(serviceUUID.toString().c_str());
                }
            }
            
            // Output the detection
            serializeJson(doc, Serial);
            Serial.println();
            
            // Log to SD card
            logDetectionToSD("ble", "raven_service_uuid", addrStr.c_str(), rssi, nullptr, name.c_str());
            
            // Update counters
            ble_detection_count++;
            total_detection_count++;
            
            // Special LED strobe for Raven (critical threat)
            ravenDetectionStrobe();
            
            if (!triggered) {
                triggered = true;
                flock_detected_beep_sequence();
            }
            // Always update detection time for heartbeat tracking
            last_detection_time = millis();
            return;
        }
    }
};

// ============================================================================
// CHANNEL HOPPING
// ============================================================================

void hop_channel()
{
    unsigned long now = millis();
    if (now - last_channel_hop > CHANNEL_HOP_INTERVAL) {
        current_channel++;
        if (current_channel > MAX_CHANNEL) {
            current_channel = 1;
        }
        esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
        last_channel_hop = now;
         printf("[WiFi] Hopped to channel %d\n", current_channel);
    }
}

// ============================================================================
// MAIN FUNCTIONS
// ============================================================================

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    printf("Starting Flock You Enhanced Detection System v2.0...\n");
    printf("ESP32-WROOM-32 DevKit V4\n\n");
    
    // Initialize LED strip
    strip.begin();
    strip.setBrightness(50);
    strip.show();
    printf("LED strip initialized (4x WS2812B)\n");
    
    // Initialize active buzzer
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    printf("Active buzzer initialized\n");
    
    // Initialize I2C for OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    printf("I2C initialized (SDA:%d, SCL:%d)\n", OLED_SDA, OLED_SCL);
    
    // Initialize OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        printf("SSD1306 allocation failed!\n");
    } else {
        printf("OLED display initialized (128x64)\n");
        displayBootScreen();
    }
    
    // Initialize GPS
    gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    printf("GPS initialized on UART2 (RX:%d, TX:%d)\n", GPS_RX, GPS_TX);
    
    // Initialize SD card
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    printf("SPI initialized for SD card\n");
    initSDCard();
    
    // Boot sequence (LED + buzzer + display)
    boot_beep_sequence();
    
    printf("\nInitializing wireless systems...\n");
    
    // Initialize WiFi in promiscuous mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
    esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
    
    printf("WiFi promiscuous mode enabled on channel %d\n", current_channel);
    printf("Monitoring probe requests and beacons...\n");
    
    // Initialize BLE
    printf("Initializing BLE scanner...\n");
    NimBLEDevice::init("");
    pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    
    printf("BLE scanner initialized\n");
    printf("\n========================================\n");
    printf("System ready - hunting for Flock Safety devices...\n");
    printf("========================================\n\n");
    
    last_channel_hop = millis();
    last_display_update = millis();
}

void loop()
{
    // Update GPS data
    updateGPS();
    
    // Handle channel hopping for WiFi promiscuous mode
    hop_channel();
    
    // Update OLED display periodically
    if (millis() - last_display_update > 1000) {
        updateDisplay();
        last_display_update = millis();
    }
    
    // Update LED breathing effect when scanning (not in detection mode)
    if (!device_in_range) {
        scanningLEDEffect();
    }
    
    // Handle heartbeat pulse if device is in range
    if (device_in_range) {
        unsigned long now = millis();
        
        // Check if 10 seconds have passed since last heartbeat
        if (now - last_heartbeat >= 10000) {
            heartbeat_pulse();
            last_heartbeat = now;
        }
        
        // Check if device has gone out of range (no detection for 30 seconds)
        if (now - last_detection_time >= 30000) {
            printf("Device out of range - stopping heartbeat\n");
            device_in_range = false;
            triggered = false; // Allow new detections
        }
    }
    
    // Handle BLE scanning
    if (millis() - last_ble_scan >= BLE_SCAN_INTERVAL && !pBLEScan->isScanning()) {
        printf("[BLE] scan...\n");
        pBLEScan->start(BLE_SCAN_DURATION, false);
        last_ble_scan = millis();
    }
    
    if (pBLEScan->isScanning() == false && millis() - last_ble_scan > BLE_SCAN_DURATION * 1000) {
        pBLEScan->clearResults();
    }
    
    delay(100);
}
