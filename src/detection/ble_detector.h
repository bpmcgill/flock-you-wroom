#ifndef BLE_DETECTOR_H
#define BLE_DETECTOR_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include "config/pins.h"
#include "config/patterns.h"

class BLEDetector {
public:
    void begin();
    void update();
    
    // Detection helpers
    static bool checkMacPrefix(const uint8_t* mac);
    static bool checkDeviceNamePattern(const char* name);

private:
    NimBLEScan* pBLEScan = nullptr;
    unsigned long lastBleScan = 0;
};

extern BLEDetector bleDetector;

#endif // BLE_DETECTOR_H
