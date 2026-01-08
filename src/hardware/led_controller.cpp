#include "led_controller.h"

// Initialize static color constants
const uint32_t LEDController::COLOR_OFF = Adafruit_NeoPixel::Color(0, 0, 0);
const uint32_t LEDController::COLOR_BLUE = Adafruit_NeoPixel::Color(0, 0, 255);
const uint32_t LEDController::COLOR_GREEN = Adafruit_NeoPixel::Color(0, 255, 0);
const uint32_t LEDController::COLOR_RED = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t LEDController::COLOR_ORANGE = Adafruit_NeoPixel::Color(255, 165, 0);
const uint32_t LEDController::COLOR_PURPLE = Adafruit_NeoPixel::Color(128, 0, 128);
const uint32_t LEDController::COLOR_WHITE = Adafruit_NeoPixel::Color(255, 255, 255);
const uint32_t LEDController::COLOR_YELLOW = Adafruit_NeoPixel::Color(255, 255, 0);

LEDController LED;

void LEDController::begin() {
    strip.begin();
    strip.setBrightness(50);
    strip.show();
}

void LEDController::setMode(LEDMode mode) {
    currentMode = mode;
}

void LEDController::setBrightness(uint8_t brightness) {
    strip.setBrightness(brightness);
}

void LEDController::setCustomFunctions(LEDFunction led0, LEDFunction led1, LEDFunction led2, LEDFunction led3) {
    customFunctions[0] = led0;
    customFunctions[1] = led1;
    customFunctions[2] = led2;
    customFunctions[3] = led3;
}

void LEDController::setLED(uint8_t index, uint32_t color) {
    if (index >= LED_COUNT) return;
    strip.setPixelColor(index, color);
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

void LEDController::knownDeviceAlert() {
    // Yellow flash for known device (not as urgent)
    flash(COLOR_YELLOW, 2, 100);
}

// ============================================================================
// MODE-SPECIFIC LED UPDATES
// ============================================================================

void LEDController::updateStatus(bool systemOK, bool wifiActive, bool bleActive, bool gpsLocked, bool sdOK) {
    if (currentMode != LED_MODE_STATUS) return;
    
    // LED 0: Power/System - Green=OK, Red=Error, Orange=Warning
    strip.setPixelColor(0, systemOK ? COLOR_GREEN : COLOR_RED);
    
    // LED 1: WiFi Detection - Blue when active
    strip.setPixelColor(1, wifiActive ? COLOR_BLUE : COLOR_OFF);
    
    // LED 2: BLE Detection - Purple when active
    strip.setPixelColor(2, bleActive ? COLOR_PURPLE : COLOR_OFF);
    
    // LED 3: GPS Status - Yellow=Locked, Orange=Searching, Off=Disabled
    strip.setPixelColor(3, gpsLocked ? COLOR_YELLOW : (sdOK ? COLOR_ORANGE : COLOR_OFF));
    
    strip.show();
}

void LEDController::updateSignalStrength(int rssi) {
    if (currentMode != LED_MODE_SIGNAL) return;
    
    // Convert RSSI to 0-4 bars
    // -90 dBm = 0 bars (very weak)
    // -30 dBm = 4 bars (very strong)
    int bars = map(constrain(rssi, -90, -30), -90, -30, 0, 4);
    
    for (int i = 0; i < LED_COUNT; i++) {
        if (i < bars) {
            // Color gradient: Red (weak) -> Yellow -> Green (strong)
            if (bars <= 1) strip.setPixelColor(i, COLOR_RED);
            else if (bars == 2) strip.setPixelColor(i, COLOR_ORANGE);
            else if (bars == 3) strip.setPixelColor(i, COLOR_YELLOW);
            else strip.setPixelColor(i, COLOR_GREEN);
        } else {
            strip.setPixelColor(i, COLOR_OFF);
        }
    }
    strip.show();
}

void LEDController::updateDetectionCount(int count) {
    if (currentMode != LED_MODE_COUNTER) return;
    
    // Milestone-based LED progression
    // LED 0: Always on after first detection
    // LED 1: Lights up after 10 detections
    // LED 2: Lights up after 50 detections
    // LED 3: Lights up after 100 detections
    
    strip.setPixelColor(0, count >= 1   ? COLOR_GREEN : COLOR_OFF);
    strip.setPixelColor(1, count >= 10  ? COLOR_BLUE : COLOR_OFF);
    strip.setPixelColor(2, count >= 50  ? COLOR_ORANGE : COLOR_OFF);
    strip.setPixelColor(3, count >= 100 ? COLOR_RED : COLOR_OFF);
    
    strip.show();
}

void LEDController::updateThreatLevel(int deviceCount) {
    if (currentMode != LED_MODE_THREAT) return;
    
    // Threat levels based on number of detected devices
    uint32_t color;
    int litLEDs;
    
    if (deviceCount == 0) {
        color = COLOR_GREEN;
        litLEDs = 2;  // LED 0-1 green
    } else if (deviceCount <= 5) {
        color = COLOR_YELLOW;
        litLEDs = 3;  // LED 0-2 yellow
    } else if (deviceCount <= 10) {
        color = COLOR_ORANGE;
        litLEDs = 3;  // LED 0-2 orange
    } else {
        color = COLOR_RED;
        litLEDs = 4;  // All LEDs red (high threat)
    }
    
    for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, i < litLEDs ? color : COLOR_OFF);
    }
    strip.show();
}

void LEDController::setLEDByFunction(uint8_t index, LEDFunction func, bool power, bool wifi, bool ble, bool gps, bool sd, bool scanning, bool detection) {
    uint32_t color = COLOR_OFF;
    
    switch (func) {
        case LED_FUNC_OFF:
            color = COLOR_OFF;
            break;
        case LED_FUNC_POWER:
            color = power ? COLOR_GREEN : COLOR_RED;
            break;
        case LED_FUNC_WIFI:
            color = wifi ? COLOR_BLUE : COLOR_OFF;
            break;
        case LED_FUNC_BLE:
            color = ble ? COLOR_PURPLE : COLOR_OFF;
            break;
        case LED_FUNC_GPS:
            color = gps ? COLOR_YELLOW : COLOR_OFF;
            break;
        case LED_FUNC_SD:
            color = sd ? COLOR_GREEN : COLOR_RED;
            break;
        case LED_FUNC_SCANNING:
            color = scanning ? COLOR_GREEN : COLOR_OFF;
            break;
        case LED_FUNC_DETECTION:
            color = detection ? COLOR_RED : COLOR_OFF;
            break;
    }
    
    strip.setPixelColor(index, color);
}

void LEDController::updateCustomMode(bool power, bool wifi, bool ble, bool gps, bool sd, bool scanning, bool detection) {
    if (currentMode != LED_MODE_CUSTOM) return;
    
    for (int i = 0; i < LED_COUNT; i++) {
        setLEDByFunction(i, customFunctions[i], power, wifi, ble, gps, sd, scanning, detection);
    }
    strip.show();
}

