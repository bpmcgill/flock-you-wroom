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
    
    // Header
    display.setCursor(0, 0);
    display.println(F("FLOCK DETECTOR v2.0"));
    
    // Status
    display.print(F("Status: "));
    if (deviceInRange) {
        display.println(F("DETECTED!"));
    } else {
        display.println(F("SCANNING"));
    }
    
    // Detection counts
    display.print(F("Detections: "));
    display.println(totalDetections);
    display.print(F("WiFi: "));
    display.print(wifiDetections);
    display.print(F("  BLE: "));
    display.println(bleDetections);
    
    // GPS status
    display.print(F("GPS: "));
    if (gpsValid) {
        display.print(lat, 2);
        display.print(F(","));
        display.println(lon, 2);
    } else {
        display.println(gpsStatus);
    }
    
    // SD status
    display.print(F("SD: "));
    display.println(sdInitialized ? F("OK") : F("ERR"));
    
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
