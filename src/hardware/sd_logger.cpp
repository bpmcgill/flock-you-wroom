#include "sd_logger.h"

SDLogger sdLogger;

bool SDLogger::begin() {
    printf("Initializing SD card...\n");
    
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    
    if (!sd.begin(SD_CS, SD_SCK_MHZ(4))) {
        printf("SD card initialization failed!\n");
        initialized = false;
        return false;
    }
    
    printf("SD card initialized successfully\n");
    initialized = true;
    
    // Create initial log file with header
    char filename[32];
    sprintf(filename, "flock_%lu.csv", millis());
    
    if (logFile.open(filename, O_WRONLY | O_CREAT | O_TRUNC)) {
        logFile.println("timestamp,protocol,detection_method,mac_address,rssi,ssid,device_name,gps_lat,gps_lon");
        logFile.close();
        printf("Created log file: %s\n", filename);
    } else {
        printf("Failed to create log file\n");
    }
    
    return true;
}

void SDLogger::logDetection(const char* protocol, const char* method, const char* mac,
                           int rssi, const char* ssid, const char* name,
                           bool gpsValid, double lat, double lon) {
    if (!initialized) return;
    
    // Create unique filename based on timestamp (one file per day)
    char filename[32];
    unsigned long hours = millis() / 3600000;
    sprintf(filename, "flock_%lu.csv", hours / 24);
    
    if (logFile.open(filename, O_WRONLY | O_CREAT | O_APPEND)) {
        logFile.print(millis());
        logFile.print(",");
        logFile.print(protocol);
        logFile.print(",");
        logFile.print(method);
        logFile.print(",");
        logFile.print(mac);
        logFile.print(",");
        logFile.print(rssi);
        logFile.print(",");
        logFile.print(ssid ? ssid : "");
        logFile.print(",");
        logFile.print(name ? name : "");
        logFile.print(",");
        if (gpsValid) {
            logFile.print(lat, 6);
            logFile.print(",");
            logFile.print(lon, 6);
        } else {
            logFile.print(",");
        }
        logFile.println();
        logFile.close();
    }
}
