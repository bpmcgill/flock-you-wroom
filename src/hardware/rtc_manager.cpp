#include "rtc_manager.h"

RTCManager rtcManager;

void RTCManager::begin() {
    printf("Initializing DS3231 RTC...\n");
    
    if (!rtc.begin()) {
        printf("ERROR: Could not find DS3231 RTC!\n");
        printf("Check wiring: SDA->GPIO21, SCL->GPIO22\n");
        initialized = false;
        return;
    }
    
    initialized = true;
    
    // Check if RTC lost power and needs time set
    if (rtc.lostPower()) {
        printf("WARNING: RTC lost power, time may be incorrect!\n");
        powerLost = true;
        // Set to compile time as fallback
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        printf("RTC set to compile time as fallback\n");
    }
    
    DateTime now = rtc.now();
    printf("RTC initialized successfully\n");
    printf("Current time: %s\n", getDateTimeString().c_str());
    printf("Temperature: %.2f°C\n", rtc.getTemperature());
}

void RTCManager::update() {
    // Currently no periodic updates needed
    // Could implement drift compensation here if needed
}

bool RTCManager::isValid() {
    if (!initialized) return false;
    
    DateTime now = rtc.now();
    // Check if year is reasonable (after 2020 and before 2100)
    return (now.year() >= 2020 && now.year() < 2100);
}

DateTime RTCManager::now() {
    if (!initialized) {
        return DateTime(2000, 1, 1, 0, 0, 0);  // Return epoch if not initialized
    }
    return rtc.now();
}

uint32_t RTCManager::unixTime() {
    if (!initialized) return 0;
    return rtc.now().unixtime();
}

String RTCManager::getTimeString() {
    if (!initialized) return "No RTC";
    
    DateTime now = rtc.now();
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", 
             now.hour(), now.minute(), now.second());
    return String(buffer);
}

String RTCManager::getDateTimeString() {
    if (!initialized) return "RTC Not Initialized";
    
    DateTime now = rtc.now();
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());
    return String(buffer);
}

void RTCManager::syncFromGPS(int year, int month, int day, int hour, int minute, int second) {
    if (!initialized) return;
    
    // Only sync if GPS time is reasonable
    if (year < 2020 || year > 2100) {
        printf("WARNING: GPS time invalid, skipping RTC sync\n");
        return;
    }
    
    DateTime gpsTime(year, month, day, hour, minute, second);
    rtc.adjust(gpsTime);
    
    powerLost = false;  // Clear power lost flag after successful sync
    lastSyncTime = millis();
    
    printf("RTC synced from GPS: %s\n", getDateTimeString().c_str());
}

void RTCManager::setDateTime(int year, int month, int day, int hour, int minute, int second) {
    if (!initialized) return;
    
    DateTime newTime(year, month, day, hour, minute, second);
    rtc.adjust(newTime);
    
    powerLost = false;
    lastSyncTime = millis();
    
    printf("RTC manually set to: %s\n", getDateTimeString().c_str());
}

String RTCManager::getStatus() {
    if (!initialized) return "RTC: Not Found";
    if (powerLost) return "RTC: Power Lost!";
    if (!isValid()) return "RTC: Invalid Time";
    return "RTC: OK";
}

bool RTCManager::hasLostPower() {
    return powerLost;
}

float RTCManager::getTemperature() {
    if (!initialized) return 0.0;
    return rtc.getTemperature();
}
