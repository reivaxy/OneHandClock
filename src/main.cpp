
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
  static const int x_offsets[13] = {0, -10, 0, 0, 0, 0, 0, 0, 0, 0, -10, -10, -10};
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
  uint8_t currentSecond = dateTime.getSecond();
  
  // Only refresh every second
  static uint8_t last_second = 255;
  if (currentSecond == last_second) {
    return;  // Skip this loop iteration if no second has passed
  }
  last_second = currentSecond;
  
  // Rotation angle reflects minutes and seconds: 6° per minute + 0.1° per second (360°/60min/60sec)
  float rotation_angle = 135.0f + (currentMinute * 6.0f) + (currentSecond * 0.1f);
  
  log_v("main: Displaying hour: %d, minute: %d, second: %d, angle: %.1f", currentHour, currentMinute, currentSecond, rotation_angle);

  // Create small sprite for the digit
  sp.createSprite(SPRITE_SIDE, SPRITE_SIDE);
  sp.fillSprite(TFT_BLACK);
  
  // Draw large digit with current hour centered on small sprite
  sp.setFont(&fonts::Font7);
  sp.setTextColor(TFT_WHITE);
  sp.setTextDatum(textdatum_t::middle_center);
  sp.drawNumber(currentHour, SPRITE_SIDE / 2 + x_offsets[currentHour], SPRITE_SIDE / 2);
  
  // Set pivot p,oint at sprite center
  sp.setPivot(SPRITE_SIDE / 2, SPRITE_SIDE / 2);
  
  // Push rotated sprite to display with zoom, rotating around screen center
  sp.pushRotateZoomWithAA(&lcd, center_x, center_y, rotation_angle, SPRITE_ZOOM, SPRITE_ZOOM, 0);
  sp.deleteSprite();

}