#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPSPlus.h>
#include "config/pins.h"

class GPSManager {
public:
    void begin();
    void update();
    bool isValid();
    double latitude();
    double longitude();
    double altitude();
    int satellites();
    String getLocation();
    String getStatus();

private:
    TinyGPSPlus gps;
    HardwareSerial gpsSerial{2}; // UART2
};

extern GPSManager gpsManager;

#endif // GPS_MANAGER_H
