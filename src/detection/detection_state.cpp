#include "detection_state.h"

DetectionState detectionState;

void DetectionState::recordDetection(bool isWiFi) {
    if (isWiFi) {
        wifiDetectionCount++;
    } else {
        bleDetectionCount++;
    }
    totalDetectionCount++;
    
    deviceInRange = true;
    lastDetectionTime = millis();
    if (!triggered) {
        triggered = true;
        lastHeartbeat = millis();
    }
}

void DetectionState::updateHeartbeat() {
    lastHeartbeat = millis();
}

bool DetectionState::shouldHeartbeat() {
    return deviceInRange && (millis() - lastHeartbeat >= 10000);
}

bool DetectionState::isDeviceOutOfRange() {
    return deviceInRange && (millis() - lastDetectionTime >= 30000);
}

void DetectionState::resetOutOfRange() {
    deviceInRange = false;
    triggered = false;
}
