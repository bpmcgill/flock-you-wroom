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
    
    // Date/Time for RTC sync
    int getYear();
    int getMonth();
    int getDay();
    int getHour();
    int getMinute();
    int getSecond();

private:
    TinyGPSPlus gps;
    HardwareSerial gpsSerial{2}; // UART2
};

extern GPSManager gpsManager;

#endif // GPS_MANAGER_H
