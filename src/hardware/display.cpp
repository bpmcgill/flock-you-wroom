#include "display.h"

Display display;

bool Display::begin() {
    Wire.begin(OLED_SDA, OLED_SCL);
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        printf("SSD1306 allocation failed!\n");
        return false;
    }
    
    printf("OLED display initialized (128x64)\n");
    return true;
}

void Display::showBootScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("FLOCK DETECTOR v2.0"));
    display.println(F(""));
    display.println(F("ESP32-WROOM-32"));
    display.println(F("DevKit V4"));
    display.println(F(""));
    display.println(F("Initializing..."));
    display.display();
}

void Display::update(bool deviceInRange, int totalDetections, int wifiDetections, int bleDetections,
                    bool gpsValid, double lat, double lon, const char* gpsStatus, bool sdInitialized) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Line 1: Header
    display.setCursor(0, 0);
    display.println(F("FLOCK DETECTOR v2.0"));
    
    // Line 2: Status
    display.print(F("Status: "));
    if (deviceInRange) {
        display.println(F("DETECTED!"));
    } else {
        display.println(F("SCANNING"));
    }
    
    // Line 3-4: Detection counts (MORE PROMINENT)
    display.setTextSize(1);
    display.print(F("Total: "));
    display.setTextSize(2);
    display.println(totalDetections);
    
    display.setTextSize(1);
    display.print(F("WiFi:"));
    display.print(wifiDetections);
    display.print(F(" BLE:"));
    display.println(bleDetections);
    
    // Line 5: GPS status
    display.print(F("GPS: "));
    if (gpsValid) {
        display.print(lat, 2);
        display.print(F(","));
        display.print(lon, 2);
    } else {
        display.print(gpsStatus);
    }
    
    // Line 6: SD status
    display.setCursor(90, 56);
    display.print(F("SD:"));
    display.print(sdInitialized ? F("OK") : F("X"));
    
    display.display();
}

void Display::showDetection(const char* deviceType, int rssi, bool gpsValid, double lat, double lon) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("ALERT!"));
    
    display.setTextSize(1);
    display.println(F(""));
    display.print(F("Type: "));
    display.println(deviceType);
    display.print(F("RSSI: "));
    display.print(rssi);
    display.println(F(" dBm"));
    
    if (gpsValid) {
        display.print(F("GPS: "));
        display.print(lat, 2);
        display.print(F(","));
        display.println(lon, 2);
    }
    
    display.display();
    delay(2000);
}
