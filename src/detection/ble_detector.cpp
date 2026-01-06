#include "ble_detector.h"
#include "raven_detector.h"
#include "detection_state.h"
#include "hardware/led_controller.h"
#include "hardware/buzzer.h"
#include "hardware/gps_manager.h"
#include "hardware/sd_logger.h"
#include <ArduinoJson.h>
#include <string.h>

BLEDetector bleDetector;

void outputBLEDetectionJSON(const char* mac, const char* name, int rssi, const char* detectionMethod);

class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
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
        
        // Check for Raven first (highest priority)
        if (RavenDetector::checkServiceUUID(advertisedDevice)) {
            RavenDetector::outputDetectionJSON(advertisedDevice, addrStr.c_str(), name.c_str(), rssi);
            
            if (!detectionState.triggered) {
                buzzer.detectionAlert();
            }
            detectionState.recordDetection(false);
            return;
        }
        
        // Check MAC prefix
        if (BLEDetector::checkMacPrefix(mac)) {
            outputBLEDetectionJSON(addrStr.c_str(), name.c_str(), rssi, "mac_prefix");
            
            if (!detectionState.triggered) {
                buzzer.detectionAlert();
            }
            detectionState.recordDetection(false);
            return;
        }
        
        // Check device name
        if (!name.empty() && BLEDetector::checkDeviceNamePattern(name.c_str())) {
            outputBLEDetectionJSON(addrStr.c_str(), name.c_str(), rssi, "device_name");
            
            if (!detectionState.triggered) {
                buzzer.detectionAlert();
            }
            detectionState.recordDetection(false);
            return;
        }
    }
};

void BLEDetector::begin() {
    printf("Initializing BLE scanner...\n");
    NimBLEDevice::init("");
    pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    printf("BLE scanner initialized\n");
}

void BLEDetector::update() {
    if (millis() - lastBleScan >= BLE_SCAN_INTERVAL && !pBLEScan->isScanning()) {
        printf("[BLE] scan...\n");
        pBLEScan->start(BLE_SCAN_DURATION, false);
        lastBleScan = millis();
    }
    
    if (pBLEScan->isScanning() == false && millis() - lastBleScan > BLE_SCAN_DURATION * 1000) {
        pBLEScan->clearResults();
    }
}

bool BLEDetector::checkMacPrefix(const uint8_t* mac) {
    char mac_str[9];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x", mac[0], mac[1], mac[2]);
    
    for (int i = 0; i < sizeof(mac_prefixes)/sizeof(mac_prefixes[0]); i++) {
        if (strncasecmp(mac_str, mac_prefixes[i], 8) == 0) {
            return true;
        }
    }
    return false;
}

bool BLEDetector::checkDeviceNamePattern(const char* name) {
    if (!name) return false;
    
    for (int i = 0; i < sizeof(device_name_patterns)/sizeof(device_name_patterns[0]); i++) {
        if (strcasestr(name, device_name_patterns[i])) {
            return true;
        }
    }
    return false;
}

void outputBLEDetectionJSON(const char* mac, const char* name, int rssi, const char* detectionMethod) {
    DynamicJsonDocument doc(2048);
    
    doc["timestamp"] = millis();
    doc["detection_time"] = String(millis() / 1000.0, 3) + "s";
    doc["protocol"] = "bluetooth_le";
    doc["detection_method"] = detectionMethod;
    doc["alert_level"] = "HIGH";
    doc["device_category"] = "FLOCK_SAFETY";
    doc["mac_address"] = mac;
    doc["rssi"] = rssi;
    
    if (name && strlen(name) > 0) {
        doc["device_name"] = name;
    }
    
    if (gpsManager.isValid()) {
        doc["gps_latitude"] = gpsManager.latitude();
        doc["gps_longitude"] = gpsManager.longitude();
        doc["gps_altitude"] = gpsManager.altitude();
        doc["gps_satellites"] = gpsManager.satellites();
    } else {
        doc["gps_status"] = gpsManager.getStatus().c_str();
    }
    
    serializeJson(doc, Serial);
    Serial.println();
    
    sdLogger.logDetection("ble", detectionMethod, mac, rssi, nullptr, name,
                         gpsManager.isValid(), gpsManager.latitude(), gpsManager.longitude());
    
    LED.flash(LEDController::COLOR_PURPLE, 1, 200);
}
