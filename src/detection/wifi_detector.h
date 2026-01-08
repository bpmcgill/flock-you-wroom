#ifndef WIFI_DETECTOR_H
#define WIFI_DETECTOR_H

#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "config/pins.h"
#include "config/patterns.h"

class WiFiDetector {
public:
    void begin();
    void hopChannel();
    void recordDetection();
    uint8_t getCurrentChannel() { return currentChannel; }
    
    // Detection helpers
    static bool checkMacPrefix(const uint8_t* mac);
    static bool checkSsidPattern(const char* ssid);

private:
    uint8_t currentChannel = 1;
    unsigned long lastChannelHop = 0;
    uint8_t priorityChannelIdx = 0;
    unsigned long lastDetection = 0;
    uint8_t channelCycleCount = 0;  // Track full channel scans
    static const uint8_t PRIORITY_CHANNELS[3];
};

extern WiFiDetector wifiDetector;

// Packet handler (must be global for ESP-IDF callback)
void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type);

#endif // WIFI_DETECTOR_H
