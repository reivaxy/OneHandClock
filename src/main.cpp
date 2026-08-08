/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-round-circular-tft-lcd-display
 */

#include <DIYables_TFT_Round.h>

#define BLACK DIYables_TFT::colorRGB(0, 0, 0)
#define RED DIYables_TFT::colorRGB(255, 0, 0)
#define BLUE DIYables_TFT::colorRGB(0, 0, 255)
#define WHITE DIYables_TFT::colorRGB(255, 255, 255)

#define PIN_RST 27 // The ESP32 pin GPIO27 connected to the RST pin of the circular TFT display
#define PIN_DC 25 // The ESP32 pin GPIO25 connected to the DC pin of the circular TFT display
#define PIN_CS 26 // The ESP32 pin GPIO26 connected to the CS pin of the circular TFT display

DIYables_TFT_GC9A01_Round TFT_display(PIN_RST, PIN_DC, PIN_CS);

int angle = 0;
int angles[] = {10, 50, 90, 130};

void setChipSelect(uint8_t val) {
    digitalWrite(PIN_RST, val);
}

// Open a raw SPI transaction — must be paired with spiEnd()
void spiBegin() {
    SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
    setChipSelect(0);
}

// Close a raw SPI transaction — must be paired with spiBegin()
void spiEnd() {
    setChipSelect(1);
    SPI.endTransaction();
}

// Raw command send — must be called within spiBegin()/spiEnd()
void writeCommand(uint8_t cmd) {
    digitalWrite(PIN_DC, 0);
    SPI.transfer(cmd);
    digitalWrite(PIN_DC, 1);
}

void setRotation(uint8_t r) {
    //Adafruit_GFX::setRotation(r);
    spiBegin();
    writeCommand(0x36);   
    SPI.transfer(r);  
    spiEnd();

}

void display() {
  // Sample temperature value
  float temperature = 26.4;
  float humidity = 64.7;
  TFT_display.fillScreen(BLACK);

  // Display temperature with degree symbol
  TFT_display.setTextColor(RED);
  TFT_display.setCursor(5, 100);  // Set cursor position (x, y)
  TFT_display.print("Temperature: ");
  TFT_display.print(temperature, 1);  // Print temperature with 1 decimal place
  TFT_display.print(char(247));
  TFT_display.println("C");

  // Display humidity
  TFT_display.setTextColor(BLUE);
  TFT_display.setCursor(30, 140);  // Set cursor position (x, y)
  TFT_display.print("Humidity: ");
  TFT_display.print(humidity, 1);  // Print humidity with 1 decimal place
  TFT_display.print("%");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("Arduino TFT LCD Display - show text and number"));

  TFT_display.begin();

  // Set the rotation (0 to 3)
  TFT_display.setRotation(0);  // Rotate screen 90 degrees
  TFT_display.setTextSize(2);  // Adjust text size as needed

  display();
}




void loop(void) {
  static unsigned long lastUpdate = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastUpdate >= 300) {
    angle = (angle + 1)%4;
    Serial.printf("angle: %d\n", angle);
    TFT_display.setRotation(angle);
    display();
    lastUpdate = currentTime;
  }
}
