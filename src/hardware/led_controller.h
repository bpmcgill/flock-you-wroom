#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config/pins.h"
#include "config/settings.h"

class LEDController {
public:
    // Color definitions
    static const uint32_t COLOR_OFF;
    static const uint32_t COLOR_BLUE;
    static const uint32_t COLOR_GREEN;
    static const uint32_t COLOR_RED;
    static const uint32_t COLOR_ORANGE;
    static const uint32_t COLOR_PURPLE;
    static const uint32_t COLOR_WHITE;
    static const uint32_t COLOR_YELLOW;

    void begin();
    void setMode(LEDMode mode);
    void setBrightness(uint8_t brightness);
    void setCustomFunctions(LEDFunction led0, LEDFunction led1, LEDFunction led2, LEDFunction led3);
    
    // Individual LED control
    void setLED(uint8_t index, uint32_t color);
    
    // Legacy unified mode (all LEDs same)
    void setAllLEDs(uint32_t color);
    void fadeIn(uint32_t color, int duration);
    void flash(uint32_t color, int count, int duration);
    void pulse(uint32_t color, int duration);
    void scanningEffect();
    void ravenDetectionStrobe();
    void knownDeviceAlert();  // Yellow flash for known devices
    
    // Mode-specific updates
    void updateStatus(bool systemOK, bool wifiActive, bool bleActive, bool gpsLocked, bool sdOK);
    void updateSignalStrength(int rssi);  // -90 to -30 dBm
    void updateDetectionCount(int count);
    void updateThreatLevel(int deviceCount);
    void updateCustomMode(bool power, bool wifi, bool ble, bool gps, bool sd, bool scanning, bool detection);

private:
    Adafruit_NeoPixel strip{LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800};
    unsigned long lastPulse = 0;
    bool pulsing = false;
    LEDMode currentMode = LED_MODE_UNIFIED;
    
    // Custom mode function assignments
    LEDFunction customFunctions[4] = {LED_FUNC_POWER, LED_FUNC_WIFI, LED_FUNC_BLE, LED_FUNC_GPS};
    
    // Helper for custom mode
    void setLEDByFunction(uint8_t index, LEDFunction func, bool power, bool wifi, bool ble, bool gps, bool sd, bool scanning, bool detection);
};

extern LEDController LED;

#endif // LED_CONTROLLER_H
