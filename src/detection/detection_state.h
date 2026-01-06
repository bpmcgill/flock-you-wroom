#ifndef DETECTION_STATE_H
#define DETECTION_STATE_H

#include <Arduino.h>

class DetectionState {
public:
    // Detection tracking
    bool triggered = false;
    bool deviceInRange = false;
    unsigned long lastDetectionTime = 0;
    unsigned long lastHeartbeat = 0;
    
    // Detection counters
    int wifiDetectionCount = 0;
    int bleDetectionCount = 0;
    int totalDetectionCount = 0;
    
    void recordDetection(bool isWiFi);
    void updateHeartbeat();
    bool shouldHeartbeat();
    bool isDeviceOutOfRange();
    void resetOutOfRange();
};

extern DetectionState detectionState;

#endif // DETECTION_STATE_H
