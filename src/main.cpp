
#include "LGFX.h"
#include "WiFiManager.h"
#include "DateTime.h"

static LGFX lcd;
static LGFX_Sprite sprite;
static LGFX_Sprite sp;
static WiFiManager wifiMgr;
static DateTime dateTime;

inline uint16_t getBackColor(int x, int y)
{
  return lcd.swap565(abs((x&31)-16)<<3, 0, abs((y&31)-16)<<3);
//return lcd.swap565(x, 0, y);
}

void setup(void)
{
  lcd.init();

  // lcd dimensions:
  log_i("main: LCD initialized - width=%d, height=%d", lcd.width(), lcd.height());

  // Note: WiFi connection happens in loop to avoid blocking startup
  // WiFi connection will retry automatically if not connected
  log_i("main: Setup complete, WiFi connection will happen in loop");

  sprite.setColorDepth(lcd.getColorDepth());
  sprite.setFont(&fonts::Font8);
  sprite.setTextColor(TFT_WHITE);
  sprite.setTextDatum(textdatum_t::middle_center);
  sprite.setCursor(0,0);
  sprite.drawNumber(3, lcd.width(), 0);

  lcd.startWrite();
}

void loop(void)
{
  static const float SPRITE_ZOOM = 3.0f;
  static const int SPRITE_SIDE = 120;
  static const int x_offsets[24] = {0, -10, 0, 0, 0, 0, 0, 0, 0, 0, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, 0, 0, 0, 0};
  static bool wifi_connected = false;
  static unsigned long last_wifi_attempt = 0;
  static bool first_attempt = true;

  // Try WiFi connection immediately on first loop, then every 30 seconds
  if (!wifi_connected && (first_attempt || (millis() - last_wifi_attempt) > 30000)) {
    first_attempt = false;
    last_wifi_attempt = millis();
    log_i("main: Attempting WiFi connection...");
    if (wifiMgr.connect()) {
      wifi_connected = true;
      // Sync time when WiFi connects
      log_i("main: WiFi connected, syncing NTP time");
      dateTime.syncNTPIfNeeded();
    } else {
      log_w("main: WiFi connection failed, will retry in 30 seconds");
    }
  }

  float center_x = lcd.width() / 2;
  float center_y = lcd.height() / 2;

  // Get current hour and minutes
  uint8_t currentHour = dateTime.getHour();
  uint8_t currentMinute = dateTime.getMinute();
  
  // Only refresh every 100ms
  static unsigned long last_refresh_time = 0;
  if (millis() - last_refresh_time < 100) {
    return;  // Skip this loop iteration if less than 100ms have passed
  }
  last_refresh_time = millis();
  
  // Rotation angle reflects minutes and seconds: 6° per minute + 0.1° per second (360°/60min/60sec)
  float rotation_angle = (currentMinute * 6.0f) ;
  
  // Create small sprite for the digit
  sp.createSprite(SPRITE_SIDE, SPRITE_SIDE);
  sp.fillSprite(TFT_BLACK);
  
  // Calculate color fade: light blue to light red through rainbow in 30 seconds, then back
  static unsigned long fade_start_time = 0;
  if (fade_start_time == 0) {
    fade_start_time = millis();
  }
  
  unsigned long elapsed = (millis() - fade_start_time) % 30000;  // 30 second cycle
  float fade_progress = elapsed / 15000.0f;  // 0-1 over 15 seconds
  
  // Clamp progress to 0-1 (first 15 sec goes 0->1, next 15 sec goes 1->0)
  if (fade_progress > 1.0f) {
    fade_progress = 2.0f - fade_progress;
  }
  
  // Hue: start at 300° (purple), end at 0° (red)
  float hue = 300.0f * (1.0f - fade_progress);  // 300 -> 0
  
  // HSV to RGB conversion with high saturation and brightness for light colors
  float s = 1.0f;  // Full saturation
  float v = 1.0f;  // Full brightness (255)
  
  float h_prime = hue / 60.0f;
  int i = (int)h_prime;
  float f = h_prime - i;
  
  float p = v * (1.0f - s);
  float q = v * (1.0f - f * s);
  float t = v * (1.0f - (1.0f - f) * s);
  
  float r, g, b;
  switch (i % 6) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    default: r = v; g = p; b = q; break;
  }
  
  uint16_t fade_color = lcd.color565((uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255));
  
  sp.setTextColor(fade_color);
  
  // Draw large digit with current hour centered on small sprite
  sp.setFont(&fonts::Font7);
  sp.setTextDatum(textdatum_t::middle_center);
  sp.drawNumber(currentHour, SPRITE_SIDE / 2 + x_offsets[currentHour], SPRITE_SIDE / 2);
  
  // Draw date at the bottom
  sp.setFont(&fonts::Font0);
  char date_str[10];
  uint8_t currentDay = dateTime.getDay();
  uint8_t currentMonth = dateTime.getMonth();
  sprintf(date_str, "%d/%d", currentDay, currentMonth);
  sp.drawString(date_str, SPRITE_SIDE / 2, SPRITE_SIDE - 30);
  
  // Set pivot p,oint at sprite center
  sp.setPivot(SPRITE_SIDE / 2, SPRITE_SIDE / 2);
  
  // Push rotated sprite to display with zoom, rotating around screen center
  sp.pushRotateZoomWithAA(&lcd, center_x, center_y, -rotation_angle, SPRITE_ZOOM, SPRITE_ZOOM, 0);
  sp.deleteSprite();

}