#include "wifi_detector.h"
#include "detection_state.h"
#include "hardware/led_controller.h"
#include "hardware/buzzer.h"
#include "hardware/gps_manager.h"
#include "hardware/sd_logger.h"
#include "hardware/data_manager.h"
#include "config/settings.h"
#include <ArduinoJson.h>
#include <string.h>

WiFiDetector wifiDetector;

// Priority channels (non-overlapping 2.4GHz channels)
const uint8_t WiFiDetector::PRIORITY_CHANNELS[3] = {1, 6, 11};

// WiFi frame structures
typedef struct {
    unsigned frame_ctrl:16;
    unsigned duration_id:16;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    unsigned sequence_ctrl:16;
    uint8_t addr4[6];
} wifi_ieee80211_mac_hdr_t;

typedef struct {
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[0];
} wifi_ieee80211_packet_t;

void outputWiFiDetectionJSON(const char* ssid, const uint8_t* mac, int rssi, 
                            const char* detectionType, uint8_t channel);

void WiFiDetector::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    
    printf("WiFi promiscuous mode enabled on channel %d\n", currentChannel);
}

void WiFiDetector::hopChannel() {
    unsigned long now = millis();
    if (now - lastChannelHop > CHANNEL_HOP_INTERVAL) {
        // Always do sequential scanning to ensure we hit all channels
        currentChannel++;
        if (currentChannel > MAX_CHANNEL) {
            currentChannel = 1;
            channelCycleCount++;
        }
        
        // When idle (no recent detections), spend extra time on priority channels
        // by revisiting them after each full cycle
        if (now - lastDetection > 10000 && channelCycleCount > 0) {
            // Every other hop, use a priority channel for extra coverage
            static bool usePriority = false;
            if (usePriority) {
                currentChannel = PRIORITY_CHANNELS[priorityChannelIdx];
                priorityChannelIdx = (priorityChannelIdx + 1) % 3;
            }
            usePriority = !usePriority;
        }
        
        esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
        lastChannelHop = now;
        printf("[WiFi] Hopped to channel %d\n", currentChannel);
    }
}

void WiFiDetector::recordDetection() {
    lastDetection = millis();
}

bool WiFiDetector::checkMacPrefix(const uint8_t* mac) {
    char mac_str[9];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x", mac[0], mac[1], mac[2]);
    
    for (int i = 0; i < sizeof(mac_prefixes)/sizeof(mac_prefixes[0]); i++) {
        if (strncasecmp(mac_str, mac_prefixes[i], 8) == 0) {
            return true;
        }
    }
    return false;
}

bool WiFiDetector::checkSsidPattern(const char* ssid) {
    if (!ssid) return false;
    
    for (int i = 0; i < sizeof(wifi_ssid_patterns)/sizeof(wifi_ssid_patterns[0]); i++) {
        if (strcasestr(ssid, wifi_ssid_patterns[i])) {
            return true;
        }
    }
    return false;
}

void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type) {
    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
    const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;
    
    // Filter out weak signals (likely too far or noise)
    const int RSSI_THRESHOLD = -85;
    if (ppkt->rx_ctrl.rssi < RSSI_THRESHOLD) {
        return;
    }
    
    uint8_t frame_type = (hdr->frame_ctrl & 0xFF) >> 2;
    if (frame_type != 0x20 && frame_type != 0x80) {
        return;
    }
    
    char ssid[33] = {0};
    uint8_t *payload = (uint8_t *)ipkt + 24;
    
    if (frame_type == 0x20) {
        payload += 0;
    } else {
        payload += 12;
    }
    
    if (payload[0] == 0 && payload[1] <= 32) {
        memcpy(ssid, &payload[2], payload[1]);
        ssid[payload[1]] = '\0';
    }
    
    // Check SSID match
    if (strlen(ssid) > 0 && WiFiDetector::checkSsidPattern(ssid)) {
        const char* detection_type = (frame_type == 0x20) ? "probe_request" : "beacon";
        outputWiFiDetectionJSON(ssid, hdr->addr2, ppkt->rx_ctrl.rssi, detection_type, wifiDetector.getCurrentChannel());
        
        wifiDetector.recordDetection();  // Track detection for adaptive hopping
        
        // Record in database and check if known device
        char mac_str[18];
        snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                 hdr->addr2[0], hdr->addr2[1], hdr->addr2[2], 
                 hdr->addr2[3], hdr->addr2[4], hdr->addr2[5]);
        
        HardwareConfig& hw = settingsManager.getHardware();
        
        double lat = (hw.enable_gps && gpsManager.isValid()) ? gpsManager.latitude() : 0.0;
        double lon = (hw.enable_gps && gpsManager.isValid()) ? gpsManager.longitude() : 0.0;
        bool isKnown = false;
        
        if (hw.enable_sd_card) {
            isKnown = dataManager.recordDetection(mac_str, "WiFi", ppkt->rx_ctrl.rssi, lat, lon);
        }
        
        if (!detectionState.triggered) {
            if (isKnown) {
                // Known device - less urgent alert
                if (hw.enable_leds) LED.knownDeviceAlert();
                if (hw.enable_buzzer) buzzer.knownDeviceBeep();
            } else {
                // New device - full alert
                if (hw.enable_buzzer || hw.enable_leds) buzzer.detectionAlert();
            }
        }
        detectionState.recordDetection(true);
        return;
    }
    
    // Check MAC match
    if (WiFiDetector::checkMacPrefix(hdr->addr2)) {
        const char* detection_type = (frame_type == 0x20) ? "probe_request_mac" : "beacon_mac";
        outputWiFiDetectionJSON(ssid[0] ? ssid : "hidden", hdr->addr2, ppkt->rx_ctrl.rssi, 
                               detection_type, wifiDetector.getCurrentChannel());
        
        wifiDetector.recordDetection();  // Track detection for adaptive hopping
        
        // Record in database and check if known device
        char mac_str[18];
        snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                 hdr->addr2[0], hdr->addr2[1], hdr->addr2[2], 
                 hdr->addr2[3], hdr->addr2[4], hdr->addr2[5]);
        
        HardwareConfig& hw = settingsManager.getHardware();
        
        double lat = (hw.enable_gps && gpsManager.isValid()) ? gpsManager.latitude() : 0.0;
        double lon = (hw.enable_gps && gpsManager.isValid()) ? gpsManager.longitude() : 0.0;
        bool isKnown = false;
        
        if (hw.enable_sd_card) {
            isKnown = dataManager.recordDetection(mac_str, "WiFi", ppkt->rx_ctrl.rssi, lat, lon);
        }
        
        if (!detectionState.triggered) {
            if (isKnown) {
                if (hw.enable_leds) LED.knownDeviceAlert();
                if (hw.enable_buzzer) buzzer.knownDeviceBeep();
            } else {
                if (hw.enable_buzzer || hw.enable_leds) buzzer.detectionAlert();
            }
        }
        detectionState.recordDetection(true);
        return;
    }
}

void outputWiFiDetectionJSON(const char* ssid, const uint8_t* mac, int rssi,
                            const char* detectionType, uint8_t channel) {
    DynamicJsonDocument doc(2048);
    
    doc["timestamp"] = millis();
    doc["detection_time"] = String(millis() / 1000.0, 3) + "s";
    doc["protocol"] = "wifi";
    doc["detection_method"] = detectionType;
    doc["alert_level"] = "HIGH";
    doc["device_category"] = "FLOCK_SAFETY";
    doc["ssid"] = ssid;
    doc["rssi"] = rssi;
    doc["channel"] = channel;
    
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    doc["mac_address"] = mac_str;
    
    if (gpsManager.isValid()) {
        doc["gps_latitude"] = gpsManager.latitude();
        doc["gps_longitude"] = gpsManager.longitude();
        doc["gps_altitude"] = gpsManager.altitude();
        doc["gps_satellites"] = gpsManager.satellites();
    } else {
        doc["gps_status"] = gpsManager.getStatus().c_str();
    }
    
    serializeJson(doc, Serial);
    Serial.println();
    
    sdLogger.logDetection("wifi", detectionType, mac_str, rssi, ssid, nullptr,
                         gpsManager.isValid(), gpsManager.latitude(), gpsManager.longitude());
    
    LED.flash(LEDController::COLOR_BLUE, 1, 200);
}
