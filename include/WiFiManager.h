#pragma once

#include <WiFi.h>

class WiFiManager {
private:
    static const int MAX_CONNECT_ATTEMPTS = 20;
    static const int CONNECT_TIMEOUT_MS = 500;
    
    bool connected;
    
public:
    WiFiManager();
    
    bool connect();
    bool isConnected();
    void disconnect();
    String getLocalIP();
    String getSSID();
};
