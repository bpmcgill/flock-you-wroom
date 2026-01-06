#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "config/pins.h"

class Display {
public:
    bool begin();
    void showBootScreen();
    void update(bool deviceInRange, int totalDetections, int wifiDetections, int bleDetections, 
                bool gpsValid, double lat, double lon, const char* gpsStatus, bool sdInitialized);
    void showDetection(const char* deviceType, int rssi, bool gpsValid, double lat, double lon);

private:
    Adafruit_SSD1306 display{SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET};
};

extern Display display;

#endif // DISPLAY_H
