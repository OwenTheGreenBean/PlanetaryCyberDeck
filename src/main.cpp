/**
 * Planet Weather Scanner — ESP32-C3 + TFT_eSPI
 * =============================================
 * Controls:
 *   Button 1 (GPIO0)  — previous planet
 *   Button 2 (GPIO1)  — next planet
 *   Pot 1    (GPIO3)  — rotate planet
 *   Pot 2    (GPIO5)  — highlight weather fields
 *   Buzzer   (GPIO8)  — startup + planet-switch tones
 */

#include <Arduino.h>
#include "Planet.h"
#include "Input.h"
#include "UI.h"
#include "Audio.h"

static InputHandler gInput;
static ScannerUI gUI;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("Planet Weather Scanner v1.5"));
  Serial.println(F("Deep-Space Exploration Division"));

  Audio::get().begin();
  gInput.begin();
  gUI.begin();

  Audio::get().playStartup();
}

void loop() {
  uint32_t now = millis();
  Audio::get().tick(now);
  gInput.update();
  gUI.update(now, gInput);
}
