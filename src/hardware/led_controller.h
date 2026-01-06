#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config/pins.h"

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

    void begin();
    void setAllLEDs(uint32_t color);
    void fadeIn(uint32_t color, int duration);
    void flash(uint32_t color, int count, int duration);
    void pulse(uint32_t color, int duration);
    void scanningEffect();
    void ravenDetectionStrobe();

private:
    Adafruit_NeoPixel strip{LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800};
    unsigned long lastPulse = 0;
    bool pulsing = false;
};

extern LEDController LED;

#endif // LED_CONTROLLER_H
