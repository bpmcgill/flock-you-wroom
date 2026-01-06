#ifndef RAVEN_DETECTOR_H
#define RAVEN_DETECTOR_H

#include <Arduino.h>
#include <NimBLEAdvertisedDevice.h>
#include "config/patterns.h"

class RavenDetector {
public:
    static bool checkServiceUUID(NimBLEAdvertisedDevice* device, char* detectedServiceOut = nullptr);
    static const char* getServiceDescription(const char* uuid);
    static const char* estimateFirmwareVersion(NimBLEAdvertisedDevice* device);
    static void outputDetectionJSON(NimBLEAdvertisedDevice* device, const char* mac, 
                                   const char* name, int rssi);
};

#endif // RAVEN_DETECTOR_H
