#pragma once

#include <stdint.h>
#include "Planet.h"
#include "Config.h"

enum class ButtonEvent : uint8_t {
  None,
  PrevPlanet,
  NextPlanet
};

/// Weather fields highlighted by Pot 2.
enum class WeatherField : uint8_t {
  Temp = 0,
  Wind,
  Pressure,
  Storm,
  Atmosphere,
  Surface,
  Log,
  COUNT
};

class InputHandler {
public:
  void begin();
  void update();

  /// Pot 1 — normalized 0..1, used to set planet rotation.
  float rotationNorm() const { return _rotationNorm; }

  /// Pot 2 — which weather field to highlight.
  WeatherField weatherField() const { return _weatherField; }
  int weatherFieldIndex() const { return static_cast<int>(_weatherField); }

  bool weatherFieldChanged() const { return _fieldChanged; }

  /// Consume one-shot button events (clears after read).
  ButtonEvent pollButton();

  void clearChangeFlags();

private:
  int mapPotToSlots(int raw, int count) const;
  float readSmoothed(int pin, float& filtered);

  float _rotationNorm = 0.0f;
  float _rotFilter = 0.0f;
  float _fieldFilter = 0.0f;

  WeatherField _weatherField = WeatherField::Temp;
  int _lastFieldSlot = -1;
  bool _fieldChanged = false;

  bool _btnPrevPrev = true;
  bool _btnNextPrev = true;
  uint32_t _btnDebounceMs = 0;
  ButtonEvent _pendingEvent = ButtonEvent::None;
};
