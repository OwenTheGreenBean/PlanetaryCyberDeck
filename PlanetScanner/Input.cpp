#include "Input.h"
#include <Arduino.h>

void InputHandler::begin() {
  pinMode(BTN_SCAN_PIN, INPUT_PULLUP);
  pinMode(BTN_MENU_PIN, INPUT_PULLUP);
  analogReadResolution(12);  // ESP32-C3 12-bit ADC

  // Seed pot slots from initial readings
  _lastPlanetSlot = mapPotToIndex(analogRead(POT_PLANET_PIN), PlanetDatabase::COUNT);
  _lastPageSlot = static_cast<int>(mapPotToPage(analogRead(POT_PAGE_PIN)));
  _planetIndex = _lastPlanetSlot;
  _infoPage = static_cast<InfoPage>(_lastPageSlot);
}

int InputHandler::mapPotToIndex(int raw, int count) const {
  // Divide ADC range into equal slots with hysteresis margins at boundaries.
  int slot = (raw * count) / 4095;
  if (slot >= count) slot = count - 1;
  if (slot < 0) slot = 0;
  return slot;
}

InfoPage InputHandler::mapPotToPage(int raw) const {
  int pages = static_cast<int>(InfoPage::PAGE_COUNT);
  int slot = (raw * pages) / 4095;
  if (slot >= pages) slot = pages - 1;
  return static_cast<InfoPage>(slot);
}

void InputHandler::update() {
  _planetChanged = false;
  _pageChanged = false;

  int planetSlot = mapPotToIndex(analogRead(POT_PLANET_PIN), PlanetDatabase::COUNT);
  if (planetSlot != _lastPlanetSlot) {
    _lastPlanetSlot = planetSlot;
    _planetIndex = planetSlot;
    _planetChanged = true;
  }

  int pageSlot = static_cast<int>(mapPotToPage(analogRead(POT_PAGE_PIN)));
  if (pageSlot != _lastPageSlot) {
    _lastPageSlot = pageSlot;
    _infoPage = static_cast<InfoPage>(pageSlot);
    _pageChanged = true;
  }

  bool scanNow = digitalRead(BTN_SCAN_PIN) == LOW;
  bool menuNow = digitalRead(BTN_MENU_PIN) == LOW;

  if (scanNow && !_btnScanPrev) _pendingEvent = ButtonEvent::ScanPressed;
  if (menuNow && !_btnMenuPrev) _pendingEvent = ButtonEvent::MenuPressed;

  _btnScanPrev = scanNow;
  _btnMenuPrev = menuNow;
}

ButtonEvent InputHandler::pollButton() {
  ButtonEvent e = _pendingEvent;
  _pendingEvent = ButtonEvent::None;
  return e;
}

void InputHandler::clearChangeFlags() {
  _planetChanged = false;
  _pageChanged = false;
}
