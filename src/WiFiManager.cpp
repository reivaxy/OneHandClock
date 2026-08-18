#include "WiFiManager.h"
#include "wifi_config.h"

WiFiManager::WiFiManager() : connected(false) {
    log_i("WiFiManager: Initialized");
}

bool WiFiManager::connect() {
    log_i("WiFiManager: Attempting to connect to SSID: %s", WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < MAX_CONNECT_ATTEMPTS) {
        delay(CONNECT_TIMEOUT_MS);
        vTaskDelay(pdMS_TO_TICKS(10)); // Feed the watchdog
        uint8_t status = WiFi.status();
        log_d("WiFiManager: Connection attempt %d/%d, status=%d", attempts + 1, MAX_CONNECT_ATTEMPTS, status);
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        connected = true;
        log_i("WiFiManager: Successfully connected!");
        log_i("WiFiManager: IP address: %s", WiFi.localIP().toString().c_str());
        log_i("WiFiManager: RSSI: %d dBm", WiFi.RSSI());
        return true;
    } else {
        connected = false;
        uint8_t status = WiFi.status();
        log_e("WiFiManager: Failed to connect after %d attempts. Final status: %d", MAX_CONNECT_ATTEMPTS, status);
        return false;
    }
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::disconnect() {
    WiFi.disconnect(true); // true = turn off WiFi radio
    connected = false;
    log_i("WiFi disconnected");
}

String WiFiManager::getLocalIP() {
    return WiFi.localIP().toString();
}

String WiFiManager::getSSID() {
    return WiFi.SSID();
}
