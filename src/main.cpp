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

// Hardware modules
#include "hardware/led_controller.h"
#include "hardware/buzzer.h"
#include "hardware/display.h"
#include "hardware/gps_manager.h"
#include "hardware/sd_logger.h"

// Detection modules
#include "detection/detection_state.h"
#include "detection/wifi_detector.h"
#include "detection/ble_detector.h"

// ============================================================================
// GLOBAL STATE
// ============================================================================

static unsigned long lastDisplayUpdate = 0;

// ============================================================================
// SETUP & LOOP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    printf("Starting Flock You Enhanced Detection System v2.0...\n");
    printf("ESP32-WROOM-32 DevKit V4\n\n");
    
    // Initialize hardware
    LED.begin();
    printf("LED strip initialized (4x WS2812B)\n");
    
    buzzer.begin();
    printf("Active buzzer initialized\n");
    
    if (display.begin()) {
        display.showBootScreen();
    }
    
    gpsManager.begin();
    sdLogger.begin();
    
    // Boot sequence (LED + buzzer + display)
    buzzer.bootSequence();
    
    printf("\nInitializing wireless systems...\n");
    
    // Initialize detection systems
    wifiDetector.begin();
    bleDetector.begin();
    
    printf("\n========================================\n");
    printf("System ready - hunting for Flock Safety devices...\n");
    printf("========================================\n\n");
    
    lastDisplayUpdate = millis();
}

void loop() {
    // Update GPS data
    gpsManager.update();
    
    // Handle WiFi channel hopping
    wifiDetector.hopChannel();
    
    // Handle BLE scanning
    bleDetector.update();
    
    // Update OLED display periodically
    if (millis() - lastDisplayUpdate > 1000) {
        display.update(
            detectionState.deviceInRange,
            detectionState.totalDetectionCount,
            detectionState.wifiDetectionCount,
            detectionState.bleDetectionCount,
            gpsManager.isValid(),
            gpsManager.latitude(),
            gpsManager.longitude(),
            gpsManager.getStatus().c_str(),
            sdLogger.isInitialized()
        );
        lastDisplayUpdate = millis();
    }
    
    // Update LED breathing effect when scanning (not in detection mode)
    if (!detectionState.deviceInRange) {
        LED.scanningEffect();
    }
    
    // Handle heartbeat pulse if device is in range
    if (detectionState.shouldHeartbeat()) {
        buzzer.heartbeat();
        detectionState.updateHeartbeat();
    }
    
    // Check if device has gone out of range
    if (detectionState.isDeviceOutOfRange()) {
        printf("Device out of range - stopping heartbeat\n");
        detectionState.resetOutOfRange();
    }
    
    delay(100);
}
