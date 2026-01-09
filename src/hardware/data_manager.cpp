#include "data_manager.h"
#include "rtc_manager.h"
#include "../config/settings.h"
#include <ArduinoJson.h>

DataManager dataManager;

void DataManager::init() {
    printf("Initializing data manager...\n");
    
    // Load existing database into memory cache
    loadDatabase();
    
    printf("Data manager initialized with %d known devices\n", device_cache.size());
}

void DataManager::loadDatabase() {
    if (!SD.exists(DB_FILE)) {
        printf("No existing database found, starting fresh\n");
        return;
    }
    
    File db = SD.open(DB_FILE, FILE_READ);
    if (!db) {
        printf("Failed to open database file\n");
        return;
    }
    
    uint32_t loaded = 0;
    while (db.available()) {
        String line = db.readStringUntil('\n');
        line.trim();
        
        if (line.length() == 0 || line.startsWith("#")) continue;
        
        // Parse: MAC,Type,RSSI,FirstSeen,LastSeen,Count
        int idx1 = line.indexOf(',');
        int idx2 = line.indexOf(',', idx1 + 1);
        int idx3 = line.indexOf(',', idx2 + 1);
        int idx4 = line.indexOf(',', idx3 + 1);
        int idx5 = line.indexOf(',', idx4 + 1);
        
        if (idx1 > 0 && idx2 > 0 && idx3 > 0 && idx4 > 0 && idx5 > 0) {
            DeviceRecord record;
            record.mac = line.substring(0, idx1);
            record.type = line.substring(idx1 + 1, idx2);
            record.rssi = line.substring(idx2 + 1, idx3).toInt();
            record.first_seen = line.substring(idx3 + 1, idx4).toInt();
            record.last_seen = line.substring(idx4 + 1, idx5).toInt();
            record.detection_count = line.substring(idx5 + 1).toInt();
            record.is_new = false;  // Existing device
            
            device_cache[record.mac] = record;
            loaded++;
        }
    }
    db.close();
    
    // Load locations
    if (SD.exists(LOCATIONS_FILE)) {
        File loc = SD.open(LOCATIONS_FILE, FILE_READ);
        if (loc) {
            while (loc.available()) {
                String line = loc.readStringUntil('\n');
                line.trim();
                
                int idx = line.indexOf(',');
                if (idx > 0) {
                    String mac = line.substring(0, idx);
                    String locations = line.substring(idx + 1);
                    device_locations[mac] = locations;
                }
            }
            loc.close();
        }
    }
    
    printf("Loaded %d devices from database\n", loaded);
}

// Helper to get timestamp - uses RTC if available, else millis()
String DataManager::getTimestamp() {
    if (settingsManager.getHardware().enable_rtc && rtcManager.isValid()) {
        return rtcManager.getDateTimeString();
    }
    // Fallback to millis
    return String(millis() / 1000.0, 3) + "s";
}

bool DataManager::recordDetection(const char* mac, const char* type, int rssi,
                                  double lat, double lon) {
    String mac_str = String(mac);
    unsigned long now = millis();
    bool is_known = device_cache.find(mac_str) != device_cache.end();
    
    if (!is_known) {
        // New device - create record
        DeviceRecord record;
        record.mac = mac_str;
        record.type = String(type);
        record.rssi = rssi;
        record.first_seen = now;
        record.last_seen = now;
        record.detection_count = 1;
        record.is_new = true;
        
        device_cache[mac_str] = record;
        new_devices_this_session++;
        
        printf("[DataMgr] NEW DEVICE: %s (%s)\n", mac, type);
    } else {
        // Known device - update record
        DeviceRecord& record = device_cache[mac_str];
        record.last_seen = now;
        record.detection_count++;
        record.rssi = rssi;  // Update to latest RSSI
        
        // Update type if we have a more specific one
        if (String(type) != "Unknown" && record.type == "Unknown") {
            record.type = String(type);
        }
        
        printf("[DataMgr] KNOWN DEVICE: %s (seen %d times)\n", mac, record.detection_count);
    }
    
    // Add location if valid
    if (lat != 0.0 && lon != 0.0) {
        addLocation(mac, lat, lon);
    }
    
    // Auto-flush check
    autoFlush();
    
    return is_known;  // Return true if this was a known device
}

void DataManager::addLocation(const char* mac, double lat, double lon) {
    char loc[30];
    snprintf(loc, sizeof(loc), "%.6f,%.6f", lat, lon);
    
    String mac_str = String(mac);
    
    if (device_locations.find(mac_str) == device_locations.end()) {
        device_locations[mac_str] = String(loc);
    } else {
        // Only add if not duplicate
        if (device_locations[mac_str].indexOf(loc) == -1) {
            device_locations[mac_str] += ";" + String(loc);
        }
    }
}

DeviceRecord* DataManager::getDevice(const char* mac) {
    auto it = device_cache.find(String(mac));
    if (it != device_cache.end()) {
        return &it->second;
    }
    return nullptr;
}

bool DataManager::isKnownDevice(const char* mac) {
    return device_cache.find(String(mac)) != device_cache.end();
}

uint32_t DataManager::getDetectionCount(const char* mac) {
    auto it = device_cache.find(String(mac));
    return (it != device_cache.end()) ? it->second.detection_count : 0;
}

void DataManager::autoFlush() {
    unsigned long now = millis();
    
    // Flush based on time or cache size
    if ((now - last_flush > FLUSH_INTERVAL) || 
        (device_cache.size() > MAX_CACHE_SIZE)) {
        flush();
    }
}

void DataManager::flush() {
    printf("[DataMgr] Flushing to SD card...\n");
    saveToDatabase();
    last_flush = millis();
}

void DataManager::saveToDatabase() {
    // Write main database
    File db = SD.open(DB_FILE, FILE_WRITE);
    if (!db) {
        printf("[DataMgr] Failed to open database for writing\n");
        return;
    }
    
    db.println("# Flock Detection Database");
    db.println("# Format: MAC,Type,RSSI,FirstSeen,LastSeen,Count");
    
    for (auto& kv : device_cache) {
        DeviceRecord& rec = kv.second;
        db.print(rec.mac);
        db.print(",");
        db.print(rec.type);
        db.print(",");
        db.print(rec.rssi);
        db.print(",");
        db.print(rec.first_seen);
        db.print(",");
        db.print(rec.last_seen);
        db.print(",");
        db.println(rec.detection_count);
    }
    db.close();
    
    // Write locations database
    File loc = SD.open(LOCATIONS_FILE, FILE_WRITE);
    if (loc) {
        for (auto& kv : device_locations) {
            loc.print(kv.first);
            loc.print(",");
            loc.println(kv.second);
        }
        loc.close();
    }
    
    // Write index
    File idx = SD.open(INDEX_FILE, FILE_WRITE);
    if (idx) {
        for (auto& kv : device_cache) {
            idx.println(kv.first);
        }
        idx.close();
    }
    
    printf("[DataMgr] Saved %d devices to database\n", device_cache.size());
}

void DataManager::exportToGeoJSON(const char* filename) {
    File file = SD.open(filename, FILE_WRITE);
    if (!file) return;
    
    file.println("{");
    file.println("  \"type\": \"FeatureCollection\",");
    file.println("  \"features\": [");
    
    bool first = true;
    for (auto& kv : device_cache) {
        String mac = kv.first;
        DeviceRecord& rec = kv.second;
        
        // Get locations for this device
        auto loc_it = device_locations.find(mac);
        if (loc_it == device_locations.end()) continue;
        
        String locations = loc_it->second;
        int pos = 0;
        
        while (pos < locations.length()) {
            int semi = locations.indexOf(';', pos);
            if (semi == -1) semi = locations.length();
            
            String loc = locations.substring(pos, semi);
            int comma = loc.indexOf(',');
            
            if (comma > 0) {
                String lat = loc.substring(0, comma);
                String lon = loc.substring(comma + 1);
                
                if (!first) file.println(",");
                first = false;
                
                file.println("    {");
                file.println("      \"type\": \"Feature\",");
                file.println("      \"geometry\": {");
                file.println("        \"type\": \"Point\",");
                file.print("        \"coordinates\": [");
                file.print(lon);
                file.print(", ");
                file.print(lat);
                file.println("]");
                file.println("      },");
                file.println("      \"properties\": {");
                file.print("        \"mac\": \"");
                file.print(mac);
                file.println("\",");
                file.print("        \"type\": \"");
                file.print(rec.type);
                file.println("\",");
                file.print("        \"rssi\": ");
                file.print(rec.rssi);
                file.println(",");
                file.print("        \"detections\": ");
                file.println(rec.detection_count);
                file.println("      }");
                file.print("    }");
            }
            
            pos = semi + 1;
        }
    }
    
    file.println();
    file.println("  ]");
    file.println("}");
    file.close();
    
    printf("[DataMgr] GeoJSON exported to %s\n", filename);
}

void DataManager::exportToCSV(const char* filename) {
    File file = SD.open(filename, FILE_WRITE);
    if (!file) return;
    
    file.println("MAC,Type,RSSI,FirstSeen,LastSeen,DetectionCount,Locations");
    
    for (auto& kv : device_cache) {
        String mac = kv.first;
        DeviceRecord& rec = kv.second;
        
        file.print(mac);
        file.print(",");
        file.print(rec.type);
        file.print(",");
        file.print(rec.rssi);
        file.print(",");
        file.print(rec.first_seen);
        file.print(",");
        file.print(rec.last_seen);
        file.print(",");
        file.print(rec.detection_count);
        file.print(",");
        
        auto loc_it = device_locations.find(mac);
        if (loc_it != device_locations.end()) {
            file.print(loc_it->second);
        }
        file.println();
    }
    
    file.close();
    printf("[DataMgr] CSV exported to %s\n", filename);
}

void DataManager::exportSummary() {
    printf("[DataMgr] Exporting all formats...\n");
    
    flush();  // Ensure database is up to date
    
    exportToGeoJSON("/export_map.geojson");
    exportToCSV("/export_data.csv");
    
    printf("[DataMgr] Export complete!\n");
}
