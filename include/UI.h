#pragma once

#include <TFT_eSPI.h>
#include "Planet.h"
#include "Config.h"
#include "Input.h"

struct OrbitBody {
  float orbitRadius;
  float orbitSpeed;
  float phase;
  float tilt;
  int size;
  uint16_t color;
  bool satellite;
  float zDepth;
};

class ScannerUI {
public:
  ScannerUI()
      : _planetA(&_tft),
        _iconSprite(&_tft),
        _miniPlanet(&_tft),
        _frame(&_tft) {}

  void begin();
  void update(uint32_t nowMs, InputHandler& input);
  void setPlanetIndex(int index, bool playSound = true);
  int currentPlanetIndex() const { return _currentIndex; }

private:
  void generateOrbiters(int planetIndex);
  void refreshTerminal();
  void buildPlanetTerminal(const PlanetData& p);
  void wrapTerminalLines();
  int terminalScrollForHighlight() const;
  bool lineMatchesField(const char* line, WeatherField field) const;
  const char* fieldLabel(WeatherField field) const;
  uint16_t terminalLineColor(const char* line, uint32_t nowMs) const;

  void renderScene(uint32_t nowMs);
  void drawStars();
  void drawHeader(const PlanetData& p, uint32_t nowMs);
  void drawBatteryIcon(uint32_t nowMs);
  void drawOrbitPaths();
  void drawOrbiters(uint32_t nowMs, bool backPass);
  void drawHotspotLabels(const PlanetData& p, float rotation, int planetIndex);
  void drawBottomPanel(uint32_t nowMs);
  void drawTerminal(uint32_t nowMs);
  void drawPlanetPicker(uint32_t nowMs);
  void pushFrame();

  TFT_eSPI _tft;
  TFT_eSprite _planetA;
  TFT_eSprite _iconSprite;
  TFT_eSprite _miniPlanet;
  TFT_eSprite _frame;

  OrbitBody _orbiters[MAX_ORBIT_BODIES];
  int _orbitCount = 0;

  uint16_t _starX[STAR_COUNT];
  uint8_t _starY[STAR_COUNT];

  char _terminalBuf[TERMINAL_BUF_SIZE];
  char _wrappedLines[MAX_TERMINAL_LINES][TERMINAL_WRAP_CHARS + 1];
  int _wrappedCount = 0;

  uint32_t _lastPlanetRenderMs = 0;

  int _currentIndex = 0;
  float _rotation = 0.0f;
  float _cloudRot = 0.0f;
  WeatherField _highlightField = WeatherField::Temp;

  uint32_t _lastFrameMs = 0;
};
