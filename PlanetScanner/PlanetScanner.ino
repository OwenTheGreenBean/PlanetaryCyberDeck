/**
 * Planet Weather Scanner — Arduino IDE entry point
 * =================================================
 * Place this folder in your Arduino sketches directory.
 * Install "TFT_eSPI" by Bodmer via Library Manager.
 * Configure TFT_eSPI using include/User_Setup.h (see SETUP.md).
 *
 * Wiring: ESP32-C3, 2.0" ST7789 240x320 SPI TFT, 2 pots, 2 buttons.
 */

#include "Config.h"
#include "Planet.h"
#include "Input.h"
#include "UI.h"

InputHandler input;
ScannerUI ui;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("Planet Weather Scanner v1.0"));

  input.begin();
  ui.begin();
}

void loop() {
  input.update();
  ui.update(millis(), input);
}
