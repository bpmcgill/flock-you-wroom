/*
 * Flock You - Enhanced Surveillance Detection System v2.0
 * ESP32-WROOM-32 DevKit V4
 * 
 * Detects Flock Safety and Raven surveillance devices via WiFi and BLE
 */

#include <Arduino.h>

// Configuration
#include "config/pins.h"
#include "config/patterns.h"
#include "config/settings.h"

// Hardware modules
#include "hardware/led_controller.h"
#include "hardware/buzzer.h"
#include "hardware/display.h"
#include "hardware/gps_manager.h"
#include "hardware/rtc_manager.h"
#include "hardware/sd_logger.h"
#include "hardware/data_manager.h"  // Database for detection tracking

// Detection modules
#include "detection/detection_state.h"
#include "detection/wifi_detector.h"
#include "detection/ble_detector.h"

// ============================================================================
// GLOBAL STATE
// ============================================================================

static unsigned long lastDisplayUpdate = 0;

// ============================================================================
// DUAL-CORE TASK FUNCTIONS
// ============================================================================

// BLE scanning task running on Core 0
void bleTask(void* parameter) {
    while(1) {
        bleDetector.update();
        vTaskDelay(50 / portTICK_PERIOD_MS);  // Small delay to prevent watchdog issues
    }
}

// ============================================================================
// SETUP & LOOP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    printf("Starting Flock You Enhanced Detection System v2.0...\n");
    printf("ESP32-WROOM-32 DevKit V4\n\n");
    
    // Try to initialize SD card first (needed for config)
    printf("Initializing SD card...\n");
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    bool sdAvailable = SD.begin(SD_CS);
    
    if (sdAvailable) {
        printf("SD card initialized\n");
        
        // Load configuration from SD card
        settingsManager.loadFromSD();
        settingsManager.printSettings();
    } else {
        printf("SD card not found - using default settings\n");
        settingsManager.loadDefaults();
    }
    
    // Get hardware configuration
    HardwareConfig& hw = settingsManager.getHardware();
    
    // Setup BOOT button for export function
    pinMode(0, INPUT_PULLUP);
    
    // Initialize hardware based on config
    if (hw.enable_leds) {
        LED.begin();
        LED.setMode(hw.led_mode);
        LED.setBrightness(hw.led_brightness);
        LED.setCustomFunctions(hw.led0_function, hw.led1_function, hw.led2_function, hw.led3_function);
        printf("LED strip initialized (4x WS2812B)\n");
        printf("LED Mode: %d, Brightness: %d\n", hw.led_mode, hw.led_brightness);
    } else {
        printf("LEDs disabled in config\n");
    }
    
    if (hw.enable_buzzer) {
        buzzer.setType(hw.buzzer_is_passive ? BUZZER_PASSIVE : BUZZER_ACTIVE);
        buzzer.begin();
        printf("Buzzer initialized (%s)\n", hw.buzzer_is_passive ? "PASSIVE/PWM" : "ACTIVE");
    } else {
        printf("Buzzer disabled in config\n");
    }
    
    if (hw.enable_oled) {
        if (display.begin()) {
            display.showBootScreen();
        }
    } else {
        printf("OLED display disabled in config\n");
    }
    
    if (hw.enable_gps) {
        gpsManager.begin();
    } else {
        printf("GPS disabled in config\n");
    }
    
    if (hw.enable_rtc) {
        rtcManager.begin();
        if (rtcManager.isValid()) {
            printf("RTC time: %s\n", rtcManager.getDateTimeString().c_str());
        }
    } else {
        printf("RTC disabled in config\n");
    }
    
    if (hw.enable_sd_card && sdAvailable) {
        sdLogger.begin();
        
        // Initialize data manager (loads database from SD card)
        dataManager.init();
    } else if (!hw.enable_sd_card) {
        printf("SD card logging disabled in config\n");
    }
    
    // Boot sequence (LED + buzzer + display)
    if (hw.enable_buzzer || hw.enable_leds) {
        buzzer.bootSequence();
    }
    
    printf("\nInitializing wireless systems...\n");
    
    // Initialize detection systems
    wifiDetector.begin();
    bleDetector.begin();
    
    // Create BLE scanning task on Core 0 (WiFi runs on Core 1)
    xTaskCreatePinnedToCore(
        bleTask,           // Task function
        "BLE_Scanner",     // Task name
        8192,              // Stack size (bytes)
        NULL,              // Parameters
        1,                 // Priority
        NULL,              // Task handle
        0                  // Core 0
    );
    printf("BLE scanner task created on Core 0\n");
    
    if (hw.enable_sd_card) {
        printf("Loaded %d known devices from database\n", dataManager.getTotalDevices());
    }
    printf("\n========================================\n");
    printf("System ready - hunting for Flock Safety devices...\n");
    printf("Dual-core mode: WiFi on Core 1, BLE on Core 0\n");
    printf("========================================\n\n");
    
    lastDisplayUpdate = millis();
}

void loop() {
    HardwareConfig& hw = settingsManager.getHardware();
    
    // Update GPS data
    if (hw.enable_gps) {
        gpsManager.update();
        
        // Sync RTC from GPS once per hour if both are enabled
        if (hw.enable_rtc && gpsManager.isValid()) {
            static unsigned long lastRTCSync = 0;
            if (millis() - lastRTCSync > 3600000) {  // 1 hour
                rtcManager.syncFromGPS(
                    gpsManager.getYear(),
                    gpsManager.getMonth(),
                    gpsManager.getDay(),
                    gpsManager.getHour(),
                    gpsManager.getMinute(),
                    gpsManager.getSecond()
                );
                lastRTCSync = millis();
            }
        }
    }
    
    // Handle WiFi channel hopping (runs on Core 1)
    wifiDetector.hopChannel();
    
    // BLE scanning now runs on Core 0 in separate task
    
    // Update OLED display periodically
    if (hw.enable_oled && millis() - lastDisplayUpdate > 1000) {
        display.update(
            detectionState.deviceInRange,
            detectionState.totalDetectionCount,
            detectionState.wifiDetectionCount,
            detectionState.bleDetectionCount,
            hw.enable_gps ? gpsManager.isValid() : false,
            hw.enable_gps ? gpsManager.latitude() : 0.0,
            hw.enable_gps ? gpsManager.longitude() : 0.0,
            hw.enable_gps ? gpsManager.getStatus().c_str() : "Disabled",
            hw.enable_sd_card ? sdLogger.isInitialized() : false
        );
        lastDisplayUpdate = millis();
    }
    
    // Update LEDs based on mode
    if (hw.enable_leds && !detectionState.deviceInRange) {
        switch (hw.led_mode) {
            case LED_MODE_UNIFIED:
                LED.scanningEffect();  // Green breathing (legacy mode)
                break;
            case LED_MODE_STATUS:
                LED.updateStatus(
                    true,  // systemOK
                    detectionState.wifiDetectionCount > 0,
                    detectionState.bleDetectionCount > 0,
                    hw.enable_gps ? gpsManager.isValid() : false,
                    hw.enable_sd_card ? sdLogger.isInitialized() : false
                );
                break;
            case LED_MODE_SIGNAL:
                // Will update on detection with RSSI
                break;
            case LED_MODE_COUNTER:
                LED.updateDetectionCount(detectionState.totalDetectionCount);
                break;
            case LED_MODE_THREAT:
                LED.updateThreatLevel(detectionState.totalDetectionCount);
                break;
            case LED_MODE_CUSTOM:
                LED.updateCustomMode(
                    true,  // power
                    detectionState.wifiDetectionCount > 0,  // wifi
                    detectionState.bleDetectionCount > 0,   // ble
                    hw.enable_gps ? gpsManager.isValid() : false,  // gps
                    hw.enable_sd_card ? sdLogger.isInitialized() : false,  // sd
                    !detectionState.deviceInRange,  // scanning
                    detectionState.deviceInRange    // detection
                );
                break;
        }
    }
    
    // Handle heartbeat pulse if device is in range
    if (detectionState.shouldHeartbeat()) {
        if (hw.enable_buzzer || hw.enable_leds) {
            buzzer.heartbeat();
        }
        detectionState.updateHeartbeat();
    }
    
    // Check if device has gone out of range
    if (detectionState.isDeviceOutOfRange()) {
        printf("Device out of range - stopping heartbeat\n");
        detectionState.resetOutOfRange();
        
        // Flush database when device goes out of range
        if (hw.enable_sd_card) {
            dataManager.flush();
        }
    }
    
    // Check for BOOT button press to export data
    if (hw.enable_sd_card) {
        static unsigned long bootButtonPress = 0;
        static bool bootButtonHeld = false;
        
        if (digitalRead(0) == LOW && !bootButtonHeld) {  // BOOT button pressed
            bootButtonPress = millis();
            bootButtonHeld = true;
        }
        else if (digitalRead(0) == HIGH && bootButtonHeld) {  // Button released
            unsigned long pressDuration = millis() - bootButtonPress;
            bootButtonHeld = false;
            
            if (pressDuration > 2000) {  // Held for 2+ seconds
                printf("\n=== EXPORTING DATA ===\n");
                if (hw.enable_leds) {
                    LED.flash(LEDController::COLOR_PURPLE, 3, 200);
                }
                dataManager.exportSummary();
                printf("Export complete! Files: /export_map.geojson, /export_data.csv\n\n");
                if (hw.enable_leds) {
                    LED.flash(LEDController::COLOR_GREEN, 2, 100);
                }
            }
        }
    }
        
    delay(100);
}
