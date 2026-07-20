#include "Input.h"
#include <Arduino.h>

void InputHandler::begin() {
  pinMode(BTN_PREV_PIN, INPUT_PULLUP);
  pinMode(BTN_NEXT_PIN, INPUT_PULLUP);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
#if defined(analogSetPinAttenuation)
  analogSetPinAttenuation(POT_ROTATE_PIN, ADC_11db);
  analogSetPinAttenuation(POT_FIELD_PIN, ADC_11db);
#endif

  _rotFilter = (float)analogRead(POT_ROTATE_PIN) / 4095.0f;
  _fieldFilter = (float)analogRead(POT_FIELD_PIN) / 4095.0f;
  _rotationNorm = _rotFilter;

  int slot = mapPotToSlots((int)(_fieldFilter * 4095.0f), WEATHER_FIELD_COUNT);
  _lastFieldSlot = slot;
  _weatherField = static_cast<WeatherField>(slot);
}

float InputHandler::readSmoothed(int pin, float& filtered) {
  float sample = (float)analogRead(pin) / 4095.0f;
  // Light EMA — pots stay responsive without ADC jitter.
  filtered = filtered * 0.75f + sample * 0.25f;
  return filtered;
}

int InputHandler::mapPotToSlots(int raw, int count) const {
  if (count <= 1) return 0;
  int slot = (raw * count) / 4095;
  if (slot >= count) slot = count - 1;
  if (slot < 0) slot = 0;
  return slot;
}

void InputHandler::update() {
  _fieldChanged = false;

  _rotationNorm = readSmoothed(POT_ROTATE_PIN, _rotFilter);

  float fieldNorm = readSmoothed(POT_FIELD_PIN, _fieldFilter);
  int fieldSlot = mapPotToSlots((int)(fieldNorm * 4095.0f + 0.5f), WEATHER_FIELD_COUNT);
  if (fieldSlot != _lastFieldSlot) {
    _lastFieldSlot = fieldSlot;
    _weatherField = static_cast<WeatherField>(fieldSlot);
    _fieldChanged = true;
  }

  // Debounce buttons (~40 ms) so one press = one planet step.
  uint32_t now = millis();
  bool prevNow = digitalRead(BTN_PREV_PIN) == LOW;
  bool nextNow = digitalRead(BTN_NEXT_PIN) == LOW;

  if (now - _btnDebounceMs >= 40) {
    if (prevNow && !_btnPrevPrev) {
      _pendingEvent = ButtonEvent::PrevPlanet;
      _btnDebounceMs = now;
    } else if (nextNow && !_btnNextPrev) {
      _pendingEvent = ButtonEvent::NextPlanet;
      _btnDebounceMs = now;
    }
  }

  _btnPrevPrev = prevNow;
  _btnNextPrev = nextNow;
}

ButtonEvent InputHandler::pollButton() {
  ButtonEvent e = _pendingEvent;
  _pendingEvent = ButtonEvent::None;
  return e;
}

void InputHandler::clearChangeFlags() {
  _fieldChanged = false;
}
