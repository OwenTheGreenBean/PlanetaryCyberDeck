/**
 * Planet Weather Scanner — ESP32-C3 + TFT_eSPI
 * =============================================
 * Retro-futuristic handheld planetary weather display.
 * See include/Config.h for wiring and SETUP.md for TFT_eSPI configuration.
 */

#include <Arduino.h>
#include "Planet.h"
#include "Input.h"
#include "UI.h"

static InputHandler gInput;
static ScannerUI gUI;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("Planet Weather Scanner v1.0"));
  Serial.println(F("Deep-Space Exploration Division"));

  gInput.begin();
  gUI.begin();
}

void loop() {
  gInput.update();
  uint32_t now = millis();
  gUI.tickTypewriter(now);
  gUI.update(now, gInput);
}
