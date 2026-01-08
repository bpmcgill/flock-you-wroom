#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <Arduino.h>
#include <map>
#include <SD.h>

// Lightweight device record for in-memory cache
struct DeviceRecord {
    String mac;
    String type;
    int rssi;
    unsigned long first_seen;
    unsigned long last_seen;
    uint32_t detection_count;
    bool is_new;  // True if first detection this session
};

class DataManager {
public:
    void init();
    
    // Record a detection and return if it's a known device
    bool recordDetection(const char* mac, const char* type, int rssi, 
                         double lat, double lon);
    
    // Get device info
    DeviceRecord* getDevice(const char* mac);
    bool isKnownDevice(const char* mac);
    uint32_t getDetectionCount(const char* mac);
    
    // Persistence
    void flush();  // Write cache to SD
    void autoFlush();  // Flush if interval exceeded
    
    // Export functions
    void exportToGeoJSON(const char* filename);
    void exportToCSV(const char* filename);
    void exportSummary();  // Export all formats
    
    // Stats
    uint32_t getTotalDevices() { return device_cache.size(); }
    uint32_t getNewDevicesThisSession() { return new_devices_this_session; }

private:
    std::map<String, DeviceRecord> device_cache;
    std::map<String, String> device_locations;  // MAC -> "lat1,lon1;lat2,lon2"
    
    const char* DB_FILE = "/detections.db";
    const char* LOCATIONS_FILE = "/locations.db";
    const char* INDEX_FILE = "/device_index.idx";
    
    const uint32_t MAX_CACHE_SIZE = 500;
    const uint32_t FLUSH_INTERVAL = 30000;  // 30 seconds
    unsigned long last_flush = 0;
    uint32_t new_devices_this_session = 0;
    
    void loadDatabase();
    void saveToDatabase();
    void addLocation(const char* mac, double lat, double lon);
};

extern DataManager dataManager;

#endif // DATA_MANAGER_H
