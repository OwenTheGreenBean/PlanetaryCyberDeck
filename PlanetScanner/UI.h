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
        _planetB(&_tft),
        _planetBlend(&_tft),
        _iconSprite(&_tft),
        _miniPlanet(&_tft),
        _frame(&_tft) {}

  void begin();
  void tickTypewriter(uint32_t nowMs);
  void update(uint32_t nowMs, InputHandler& input);
  void setPlanetIndex(int index);
  int currentPlanetIndex() const { return _currentIndex; }

private:
  void generateOrbiters(int planetIndex);
  void tickAutoPlanet(uint32_t nowMs);
  void resetTypewriter(uint32_t nowMs, bool menuMode);
  void buildPlanetTerminal(const PlanetData& p);
  void buildMenuTerminal();
  void renderScene(uint32_t nowMs);
  void drawStars();
  void drawHeader(const PlanetData& p);
  void drawOrbitPaths();
  void drawOrbiters(uint32_t nowMs, bool backPass);
  void drawHotspotLabels(const PlanetData& p, float rotation, int planetIndex);
  void drawBottomPanel(uint32_t nowMs);
  void drawTerminal(uint32_t nowMs);
  void drawPlanetPicker(uint32_t nowMs);
  void drawScanEffect(uint32_t nowMs);
  void pushFrame();

  TFT_eSPI _tft;
  TFT_eSprite _planetA;
  TFT_eSprite _planetB;
  TFT_eSprite _planetBlend;
  TFT_eSprite _iconSprite;
  TFT_eSprite _miniPlanet;
  TFT_eSprite _frame;

  OrbitBody _orbiters[MAX_ORBIT_BODIES];
  int _orbitCount = 0;

  uint16_t _starX[STAR_COUNT];
  uint8_t _starY[STAR_COUNT];

  char _terminalBuf[TERMINAL_BUF_SIZE];
  int _terminalLen = 0;
  int _typedChars = 0;
  uint32_t _typeNextMs = 0;
  uint32_t _lastPlanetRenderMs = 0;
  bool _needsRedraw = false;

  int _currentIndex = 0;
  float _rotation = 0.0f;
  float _cloudRot = 0.0f;

  uint32_t _lastAutoSwitchMs = 0;
  uint32_t _lastFrameMs = 0;
  bool _scanActive = false;
  uint32_t _scanStartMs = 0;
  bool _menuActive = false;
  uint32_t _menuStartMs = 0;
  uint32_t _frameCount = 0;
};
