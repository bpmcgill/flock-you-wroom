#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>
#include "config/pins.h"

class SDLogger {
public:
    bool begin();
    void logDetection(const char* protocol, const char* method, const char* mac,
                     int rssi, const char* ssid, const char* name,
                     bool gpsValid, double lat, double lon);
    bool isInitialized() { return initialized; }

private:
    SdFat sd;
    SdFile logFile;
    bool initialized = false;
};

extern SDLogger sdLogger;

#endif // SD_LOGGER_H
