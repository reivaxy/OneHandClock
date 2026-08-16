#pragma once

#include <ctime>
#include <cstdint>
#include <arduino.h>

class DateTime {
private:
    uint32_t lastNTPRefresh;
    uint32_t lastMillis;
    time_t baseTime;
    static const uint32_t NTP_REFRESH_INTERVAL = 3600000; // 1 hour in milliseconds
    static const char* NTP_SERVER;
    static const char* TIMEZONE;
    
    void syncNTP();
    time_t getCurrentTime();
    
public:
    DateTime();
    
    uint8_t getHour();
    uint8_t getMinute();
    uint8_t getSecond();
    uint8_t getDay();
    uint8_t getMonth();
    uint16_t getYear();
    
    void update();
    bool syncNTPIfNeeded();
};
