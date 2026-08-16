#include "DateTime.h"

// Configure these for your timezone
const char* DateTime::NTP_SERVER = "pool.ntp.org";
const char* DateTime::TIMEZONE = "CET";

DateTime::DateTime() : lastNTPRefresh(0), lastMillis(0), baseTime(0) {
    log_i("DateTime: Initialized with default time (2024-01-01 00:00:00)");
    log_i("DateTime: Waiting for WiFi to sync time via NTP");
    
    // Set a default time (2024-01-01 00:00:00) in case NTP fails
    struct tm timeinfo = {};
    timeinfo.tm_year = 124; // 2024 - 1900
    timeinfo.tm_mon = 0;    // January
    timeinfo.tm_mday = 1;   // 1st
    baseTime = mktime(&timeinfo);
    lastMillis = millis();
}

void DateTime::syncNTP() {
    log_i("DateTime: Starting NTP sync with server: %s, timezone: CET (UTC+1)", NTP_SERVER);
    
    // Configure time with NTP server
    // gmtOffset_sec: 1*3600 (CET is UTC+1)
    // daylightOffset_sec: 1*3600 (CEST is UTC+2, which is +1 hour more than standard)
    configTime(1*3600, 1*3600, NTP_SERVER, "time.nist.gov", "time.google.com");
    
    // Wait for time to be set (max 10 seconds) - feed watchdog
    time_t now = time(nullptr);
    int attempts = 0;
    while (now < 24 * 3600 && attempts < 20) {
        delay(500);
        vTaskDelay(pdMS_TO_TICKS(10)); // Feed the watchdog
        now = time(nullptr);
        attempts++;
        if (attempts % 5 == 0) {  // Log every 5 attempts
            log_d("DateTime: NTP sync attempt %d/20, time=%ld", attempts, now);
        }
    }
    
    if (now > 24 * 3600) {
        struct tm* timeinfo = localtime(&now);
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        log_i("DateTime: NTP sync successful! Time: %s", buffer);
    } else {
        log_e("DateTime: NTP sync failed after 10 seconds");
    }
    
    baseTime = now;
    lastNTPRefresh = millis();
    lastMillis = millis();
    
    struct tm* timeinfo = localtime(&baseTime);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    log_i("DateTime: Base time set to: %s", buffer);
}

time_t DateTime::getCurrentTime() {
    // Check if refresh is needed
    if (baseTime == 0 || (millis() - lastNTPRefresh >= NTP_REFRESH_INTERVAL)) {
        log_i("DateTime: Time refresh needed, syncing NTP");
        syncNTP();
    }
    
    // Calculate current time based on elapsed milliseconds
    uint32_t elapsedMs = millis() - lastMillis;
    time_t currentTime = baseTime + (elapsedMs / 1000);
    
    return currentTime;
}

uint8_t DateTime::getHour() {
    time_t now = getCurrentTime();
    struct tm* timeinfo = localtime(&now);
    uint8_t hour = timeinfo->tm_hour;
    
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    log_d("DateTime: getHour() = %d (full time: %s)", hour, buffer);
    
    return hour;
}

uint8_t DateTime::getMinute() {
    time_t now = getCurrentTime();
    struct tm* timeinfo = localtime(&now);
    return timeinfo->tm_min;
}

uint8_t DateTime::getSecond() {
    time_t now = getCurrentTime();
    struct tm* timeinfo = localtime(&now);
    return timeinfo->tm_sec;
}

uint8_t DateTime::getDay() {
    time_t now = getCurrentTime();
    struct tm* timeinfo = localtime(&now);
    return timeinfo->tm_mday;
}

uint8_t DateTime::getMonth() {
    time_t now = getCurrentTime();
    struct tm* timeinfo = localtime(&now);
    return timeinfo->tm_mon + 1; // tm_mon is 0-based
}

uint16_t DateTime::getYear() {
    time_t now = getCurrentTime();
    struct tm* timeinfo = localtime(&now);
    return timeinfo->tm_year + 1900; // tm_year is years since 1900
}

void DateTime::update() {
    // This can be called periodically if needed, but time updates automatically
    // through getCurrentTime() checks
}

bool DateTime::syncNTPIfNeeded() {
    // Only sync if we haven't synced yet or if it's been more than an hour
    if (lastNTPRefresh == 0 || (millis() - lastNTPRefresh >= NTP_REFRESH_INTERVAL)) {
        log_i("DateTime: syncNTPIfNeeded() - syncing now (lastNTPRefresh=%lu, millis=%lu)", lastNTPRefresh, millis());
        syncNTP();
        return true;
    }
    log_d("DateTime: syncNTPIfNeeded() - sync not needed yet");
    return false;
}
