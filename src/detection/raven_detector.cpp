#include "raven_detector.h"
#include "hardware/led_controller.h"
#include "hardware/gps_manager.h"
#include "hardware/sd_logger.h"
#include <ArduinoJson.h>
#include <string.h>

bool RavenDetector::checkServiceUUID(NimBLEAdvertisedDevice* device, char* detectedServiceOut) {
    if (!device || !device->haveServiceUUID()) return false;
    
    int serviceCount = device->getServiceUUIDCount();
    if (serviceCount == 0) return false;
    
    for (int i = 0; i < serviceCount; i++) {
        NimBLEUUID serviceUUID = device->getServiceUUID(i);
        std::string uuidStr = serviceUUID.toString();
        
        for (int j = 0; j < sizeof(raven_service_uuids)/sizeof(raven_service_uuids[0]); j++) {
            if (strcasecmp(uuidStr.c_str(), raven_service_uuids[j]) == 0) {
                if (detectedServiceOut != nullptr) {
                    strncpy(detectedServiceOut, uuidStr.c_str(), 40);
                }
                return true;
            }
        }
    }
    
    return false;
}

const char* RavenDetector::getServiceDescription(const char* uuid) {
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

const char* RavenDetector::estimateFirmwareVersion(NimBLEAdvertisedDevice* device) {
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
    
    if (has_old_location && !has_new_gps)
        return "1.1.x (Legacy)";
    if (has_new_gps && !has_power_service)
        return "1.2.x";
    if (has_new_gps && has_power_service)
        return "1.3.x (Latest)";
    
    return "Unknown Version";
}

void RavenDetector::outputDetectionJSON(NimBLEAdvertisedDevice* device, const char* mac,
                                       const char* name, int rssi) {
    char detected_service_uuid[41] = {0};
    checkServiceUUID(device, detected_service_uuid);
    
    const char* fw_version = estimateFirmwareVersion(device);
    const char* service_desc = getServiceDescription(detected_service_uuid);
    
    DynamicJsonDocument doc(2048);
    doc["timestamp"] = millis();
    doc["detection_time"] = String(millis() / 1000.0, 3) + "s";
    doc["protocol"] = "bluetooth_le";
    doc["detection_method"] = "raven_service_uuid";
    doc["device_type"] = "RAVEN_GUNSHOT_DETECTOR";
    doc["manufacturer"] = "SoundThinking/ShotSpotter";
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
    
    doc["raven_service_uuid"] = detected_service_uuid;
    doc["raven_service_description"] = service_desc;
    doc["raven_firmware_version"] = fw_version;
    doc["threat_level"] = "CRITICAL";
    doc["threat_score"] = 100;
    
    if (device->haveServiceUUID()) {
        JsonArray services = doc.createNestedArray("service_uuids");
        int serviceCount = device->getServiceUUIDCount();
        for (int i = 0; i < serviceCount; i++) {
            NimBLEUUID serviceUUID = device->getServiceUUID(i);
            services.add(serviceUUID.toString().c_str());
        }
    }
    
    serializeJson(doc, Serial);
    Serial.println();
    
    sdLogger.logDetection("ble", "raven_service_uuid", mac, rssi, nullptr, name,
                         gpsManager.isValid(), gpsManager.latitude(), gpsManager.longitude());
    
    LED.ravenDetectionStrobe();
}
