#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include <RTClib.h>

class RTCManager {
public:
    void begin();
    void update();
    bool isValid();
    
    // Get current time
    DateTime now();
    uint32_t unixTime();
    String getTimeString();
    String getDateTimeString();
    
    // Sync RTC from GPS
    void syncFromGPS(int year, int month, int day, int hour, int minute, int second);
    
    // Manual time setting
    void setDateTime(int year, int month, int day, int hour, int minute, int second);
    
    // Status
    String getStatus();
    bool hasLostPower();
    float getTemperature();

private:
    RTC_DS3231 rtc;
    bool initialized = false;
    bool powerLost = false;
    unsigned long lastSyncTime = 0;
};

extern RTCManager rtcManager;

#endif // RTC_MANAGER_H
