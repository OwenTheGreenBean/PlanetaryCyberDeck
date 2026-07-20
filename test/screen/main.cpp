/**
 * Quick TFT Screen Test — ESP32-C3 + 2.0" ST7789 (240×320)
 * Cycles solid colors, draws text/border, then animates a bar.
 * Upload: pio run -e screen_test -t upload
 */

#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

static void drawPassLabel(const char* label, uint16_t fg, uint16_t bg) {
  tft.fillScreen(bg);
  tft.drawRect(0, 0, tft.width(), tft.height(), TFT_WHITE);
  tft.drawRect(2, 2, tft.width() - 4, tft.height() - 4, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(fg, bg);
  tft.drawString("DISPLAY TEST", tft.width() / 2, tft.height() / 2 - 20, 4);
  tft.drawString(label, tft.width() / 2, tft.height() / 2 + 10, 2);
  tft.drawString("PlanetWeather Scanner", tft.width() / 2, tft.height() / 2 + 36, 2);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== TFT Screen Test ===");

  Serial.print("Calling tft.init()... ");
  tft.init();
  Serial.println("OK");
  tft.setRotation(0);
  Serial.printf("TFT %dx%d rotation=0\n", tft.width(), tft.height());

  const struct { uint16_t color; const char* name; } passes[] = {
    {TFT_RED, "RED"},
    {TFT_GREEN, "GREEN"},
    {TFT_BLUE, "BLUE"},
    {TFT_WHITE, "WHITE"},
    {TFT_BLACK, "BLACK"},
    {0xFD20, "ORANGE"},
    {0x07FF, "CYAN"},
    {0x1082, "CHARCOAL"},
  };

  for (auto& p : passes) {
    Serial.printf("PASS: %s (0x%04X)\n", p.name, p.color);
    drawPassLabel(p.name, p.color == TFT_WHITE ? TFT_BLACK : TFT_WHITE, p.color);
    delay(800);
  }

  // Gradient bar animation — confirms SPI keeps working
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("ANIMATION OK", tft.width() / 2, 8, 2);
  Serial.println("PASS: animation bar");
}

void loop() {
  static int bar = 0;
  int w = tft.width() - 20;
  tft.fillRect(10, tft.height() / 2 - 4, w, 8, TFT_BLACK);
  tft.fillRect(10, tft.height() / 2 - 4, bar, 8, TFT_CYAN);
  bar += 4;
  if (bar > w) bar = 0;
  delay(30);
}
