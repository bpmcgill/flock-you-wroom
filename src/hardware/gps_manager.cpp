#include "gps_manager.h"

GPSManager gpsManager;

void GPSManager::begin() {
    gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    printf("GPS initialized on UART2 (RX:%d, TX:%d)\n", GPS_RX, GPS_TX);
}

void GPSManager::update() {
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }
}

bool GPSManager::isValid() {
    return gps.location.isValid();
}

double GPSManager::latitude() {
    return gps.location.lat();
}

double GPSManager::longitude() {
    return gps.location.lng();
}

double GPSManager::altitude() {
    return gps.altitude.meters();
}

int GPSManager::satellites() {
    return gps.satellites.value();
}

String GPSManager::getLocation() {
    if (gps.location.isValid()) {
        return String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
    }
    return "NO_FIX";
}

String GPSManager::getStatus() {
    if (gps.location.isValid()) {
        return "FIX";
    } else if (gps.satellites.value() > 0) {
        return "SEARCHING";
    }
    return "NO_GPS";
}
