#include "led_controller.h"

// Initialize static color constants
const uint32_t LEDController::COLOR_OFF = Adafruit_NeoPixel::Color(0, 0, 0);
const uint32_t LEDController::COLOR_BLUE = Adafruit_NeoPixel::Color(0, 0, 255);
const uint32_t LEDController::COLOR_GREEN = Adafruit_NeoPixel::Color(0, 255, 0);
const uint32_t LEDController::COLOR_RED = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t LEDController::COLOR_ORANGE = Adafruit_NeoPixel::Color(255, 165, 0);
const uint32_t LEDController::COLOR_PURPLE = Adafruit_NeoPixel::Color(128, 0, 128);
const uint32_t LEDController::COLOR_WHITE = Adafruit_NeoPixel::Color(255, 255, 255);

LEDController LED;

void LEDController::begin() {
    strip.begin();
    strip.setBrightness(50);
    strip.show();
}

void LEDController::setAllLEDs(uint32_t color) {
    for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, color);
    }
    strip.show();
}

void LEDController::fadeIn(uint32_t color, int duration) {
    for (int brightness = 0; brightness <= 50; brightness += 5) {
        strip.setBrightness(brightness);
        setAllLEDs(color);
        delay(duration / 10);
    }
    strip.setBrightness(50);
}

void LEDController::flash(uint32_t color, int count, int duration) {
    for (int i = 0; i < count; i++) {
        setAllLEDs(color);
        delay(duration);
        setAllLEDs(COLOR_OFF);
        if (i < count - 1) delay(duration);
    }
}

void LEDController::pulse(uint32_t color, int duration) {
    // Breathing effect
    for (int brightness = 10; brightness <= 50; brightness += 5) {
        strip.setBrightness(brightness);
        setAllLEDs(color);
        delay(duration / 16);
    }
    for (int brightness = 50; brightness >= 10; brightness -= 5) {
        strip.setBrightness(brightness);
        setAllLEDs(color);
        delay(duration / 16);
    }
    strip.setBrightness(50);
    setAllLEDs(COLOR_OFF);
}

void LEDController::scanningEffect() {
    if (!pulsing && millis() - lastPulse > 3000) {
        pulsing = true;
        pulse(COLOR_GREEN, 500);
        pulsing = false;
        lastPulse = millis();
    }
}

void LEDController::ravenDetectionStrobe() {
    // Red and white strobe for critical threat
    for (int i = 0; i < 5; i++) {
        setAllLEDs(COLOR_RED);
        delay(100);
        setAllLEDs(COLOR_WHITE);
        delay(100);
    }
    setAllLEDs(COLOR_OFF);
}
