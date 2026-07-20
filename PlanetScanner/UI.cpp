#include "UI.h"
#include "Renderer.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

void ScannerUI::begin() {
  _tft.init();
  _tft.setRotation(0);
  _tft.fillScreen(TFT_BLACK);

  _planetA.setColorDepth(16);
  _planetB.setColorDepth(16);
  _planetBlend.setColorDepth(16);
  _iconSprite.setColorDepth(16);
  _miniPlanet.setColorDepth(16);
  _frame.setColorDepth(16);

  _planetA.createSprite(PLANET_SIZE, PLANET_SIZE);
  _planetB.createSprite(PLANET_SIZE, PLANET_SIZE);
  _planetBlend.createSprite(PLANET_SIZE, PLANET_SIZE);
  _iconSprite.createSprite(24, 24);
  _miniPlanet.createSprite(MINI_PLANET_SIZE, MINI_PLANET_SIZE);
  _frame.createSprite(SCREEN_W, SCREEN_H);

  for (int i = 0; i < STAR_COUNT; i++) {
    uint32_t h = 1103515245u * (uint32_t)(i + 1) + 12345u;
    _starX[i] = h % SCREEN_W;
    _starY[i] = (uint8_t)((h >> 8) % (INFO_Y - 28));
    if (_starY[i] < HEADER_Y + 14) _starY[i] = HEADER_Y + 14;
  }

  _currentIndex = PlanetDatabase::randomIndex();
  generateOrbiters(_currentIndex);
  resetTypewriter(millis(), false);
  _lastAutoSwitchMs = millis();
  _lastFrameMs = _lastAutoSwitchMs;
}

void ScannerUI::generateOrbiters(int planetIndex) {
  randomSeed(planetIndex * 7919u + 42u);
  _orbitCount = 2 + (random(0, 65535) % 4);

  const PlanetData& p = PlanetDatabase::get(planetIndex);
  const int minOrbit = PLANET_SIZE / 2 + 16;

  for (int i = 0; i < _orbitCount; i++) {
    OrbitBody& o = _orbiters[i];
    o.orbitRadius = minOrbit + i * 14 + (random(0, 65535) % 12);
    o.orbitSpeed = 0.35f + (random(0, 65535) % 100) / 120.0f;
    o.phase = (random(0, 65535) % 628) / 100.0f;
    o.tilt = 0.35f + (random(0, 65535) % 200) / 400.0f;
    o.size = 3 + (random(0, 65535) % 3);
    o.satellite = (random(0, 65535) % 3) == 0;
    if (o.satellite) o.size += 1;
    o.color = (i % 2 == 0) ? p.colorPrimary : p.colorAccent;
    if (o.color == TFT_BLACK) o.color = COL_TEXT_DIM;
    o.zDepth = 0;
  }
}

void ScannerUI::buildPlanetTerminal(const PlanetData& p) {
  snprintf(_terminalBuf, sizeof(_terminalBuf),
           "> SCAN INIT...\n"
           "> TARGET: %s\n"
           "> TEMP: %s\n"
           "> WIND: %s\n"
           "> PRESS: %s\n"
           "> STORM: %s\n"
           "> ATM: %s\n"
           "> SURF: %s\n"
           "> LOG: %s\n"
           "> STATUS: READY",
           p.name, p.temperature, p.windSpeed, p.pressure,
           p.stormType, p.atmosphere, p.surfaceDesc, p.summary);
  _terminalLen = (int)strlen(_terminalBuf);
}

void ScannerUI::buildMenuTerminal() {
  snprintf(_terminalBuf, sizeof(_terminalBuf),
           "> SYS MENU\n"
           "> AUTO-SCAN: ON\n"
           "> ORBIT TRACK: ON\n"
           "> FIRMWARE: 1.3\n"
           "> SELECT: POT A\n"
           "> STATUS: OK");
  _terminalLen = (int)strlen(_terminalBuf);
}

void ScannerUI::resetTypewriter(uint32_t nowMs, bool menuMode) {
  if (menuMode) {
    buildMenuTerminal();
  } else {
    buildPlanetTerminal(PlanetDatabase::get(_currentIndex));
  }
  _typedChars = 0;
  _typeNextMs = nowMs;
  _needsRedraw = true;
}

void ScannerUI::tickTypewriter(uint32_t nowMs) {
  if (_typedChars >= _terminalLen) return;
  if (nowMs < _typeNextMs) return;

  _typedChars++;
  uint32_t delay = TYPE_CHAR_MS;
  if (_typedChars > 0 && _terminalBuf[_typedChars - 1] == '\n') {
    delay = TYPE_LINE_PAUSE_MS;
  }
  _typeNextMs = nowMs + delay;
  _needsRedraw = true;
}

void ScannerUI::setPlanetIndex(int index) {
  index = PlanetDatabase::wrapIndex(index);
  if (index == _currentIndex) return;

  _currentIndex = index;
  generateOrbiters(index);
  _menuActive = false;
  _lastPlanetRenderMs = 0;
  resetTypewriter(millis(), false);
  _lastAutoSwitchMs = millis();
}

void ScannerUI::tickAutoPlanet(uint32_t nowMs) {
  if (_scanActive) return;
  if (nowMs - _lastAutoSwitchMs >= AUTO_PLANET_MS) {
    setPlanetIndex(PlanetDatabase::randomIndex(_currentIndex));
    _lastAutoSwitchMs = nowMs;
  }
}

void ScannerUI::update(uint32_t nowMs, InputHandler& input) {
  bool forceDraw = _needsRedraw;
  _needsRedraw = false;

  if (!forceDraw && (nowMs - _lastFrameMs < FRAME_MS)) return;
  _lastFrameMs = nowMs;
  _frameCount++;

  if (input.planetChanged()) {
    setPlanetIndex(input.planetIndex());
    input.clearChangeFlags();
  }

  ButtonEvent btn = input.pollButton();
  if (btn == ButtonEvent::ScanPressed) {
    _scanActive = true;
    _scanStartMs = nowMs;
  } else if (btn == ButtonEvent::MenuPressed) {
    _menuActive = true;
    _menuStartMs = nowMs;
    resetTypewriter(nowMs, true);
  }

  tickAutoPlanet(nowMs);

  const PlanetData& cur = PlanetDatabase::get(_currentIndex);
  _rotation += cur.rotationSpeed * ROTATION_SCALE;
  _cloudRot += cur.cloudSpeed * ROTATION_SCALE;

  if (_scanActive && nowMs - _scanStartMs > SCAN_ANIM_MS) _scanActive = false;
  if (_menuActive && nowMs - _menuStartMs > 8000) {
    _menuActive = false;
    resetTypewriter(nowMs, false);
  }

  _frame.fillSprite(TFT_BLACK);
  renderScene(nowMs);
  drawBottomPanel(nowMs);

  if (_scanActive) drawScanEffect(nowMs);

  pushFrame();
}

void ScannerUI::drawStars() {
  for (int i = 0; i < STAR_COUNT; i++) {
    uint16_t c = (i % 7 == 0) ? COL_TEXT_DIM : 0x0180;
    _frame.drawPixel(_starX[i], _starY[i], c);
  }
}

void ScannerUI::renderScene(uint32_t nowMs) {
  const PlanetData& p = PlanetDatabase::get(_currentIndex);

  drawStars();
  drawHeader(p);

  const int px = PLANET_CX - PLANET_SIZE / 2;
  const int py = PLANET_CY - PLANET_SIZE / 2;

  drawOrbitPaths();
  drawOrbiters(nowMs, true);

  bool typing = (_typedChars < _terminalLen);
  bool redrawPlanet = !typing || (nowMs - _lastPlanetRenderMs >= PLANET_REDRAW_MS);
  if (redrawPlanet) {
    PlanetRenderer::drawPlanet(_planetA, p, _rotation, _cloudRot, nowMs);
    _lastPlanetRenderMs = nowMs;
  }
  _planetA.pushToSprite(&_frame, px, py);

  drawHotspotLabels(p, _rotation, _currentIndex);
  drawOrbiters(nowMs, false);
}

void ScannerUI::drawOrbitPaths() {
  const int pcx = PLANET_CX;
  const int pcy = PLANET_CY;

  for (int i = 0; i < _orbitCount; i++) {
    const OrbitBody& o = _orbiters[i];
    const int r = (int)o.orbitRadius;
    const float ryScale = sinf(o.tilt) * 0.42f;

    for (int step = 0; step < 360; step += 12) {
      float ang = step * (PI / 180.0f);
      int x0 = pcx + (int)(cosf(ang) * r);
      int y0 = pcy + (int)(sinf(ang) * r * ryScale);
      float ang2 = (step + 12) * (PI / 180.0f);
      int x1 = pcx + (int)(cosf(ang2) * r);
      int y1 = pcy + (int)(sinf(ang2) * r * ryScale);
      _frame.drawLine(x0, y0, x1, y1, COL_TEXT_DIM);
    }
  }
}

void ScannerUI::drawOrbiters(uint32_t nowMs, bool backPass) {
  const float t = nowMs * 0.001f;
  const int pcx = PLANET_CX;
  const int pcy = PLANET_CY;

  for (int i = 0; i < _orbitCount; i++) {
    OrbitBody& o = _orbiters[i];
    float a = o.phase + t * o.orbitSpeed;
    o.zDepth = sinf(a) * cosf(o.tilt);

    if (backPass != (o.zDepth < 0.0f)) continue;

    float ox = cosf(a) * o.orbitRadius;
    float oy = sinf(a) * o.orbitRadius * sinf(o.tilt) * 0.42f;
    int bx = pcx + (int)ox;
    int by = pcy + (int)oy;

    PlanetRenderer::drawLitSphere(_frame, bx, by, o.size, o.color, o.satellite);
  }
}

void ScannerUI::drawHotspotLabels(const PlanetData& p, float rotation, int planetIndex) {
  const WeatherHotspot* spots = PlanetDatabase::getHotspots(planetIndex);
  const int pr = PLANET_SIZE / 2 - 2;

  for (int i = 0; i < HOTSPOTS_PER_PLANET; i++) {
    int sx, sy;
    if (!PlanetRenderer::projectPoint(spots[i].lon, spots[i].lat, rotation,
                                     PLANET_CX, PLANET_CY, pr, &sx, &sy))
      continue;

    int lx = sx + ((i == 0) ? -52 : (i == 1) ? 52 : 0);
    int ly = sy + ((i == 2) ? -22 : 18);

    if (lx < 4) lx = 4;
    if (lx > SCREEN_W - 44) lx = SCREEN_W - 44;
    if (ly < HEADER_Y + 20) ly = HEADER_Y + 20;
    if (ly > INFO_Y - 14) ly = INFO_Y - 14;

    _frame.drawLine(lx, ly, sx, sy, COL_TEXT_DIM);
    _frame.fillCircle(sx, sy, 2, COL_TEXT_HI);

    _frame.setTextColor(COL_TEXT, COL_BG);
    _frame.setTextDatum(
        (i == 0) ? TL_DATUM : (i == 1) ? TR_DATUM : TC_DATUM);
    _frame.drawString(spots[i].label, lx, ly, 1);
  }

  (void)p;
}

void ScannerUI::drawBottomPanel(uint32_t nowMs) {
  drawTerminal(nowMs);
  drawPlanetPicker(nowMs);
}

void ScannerUI::drawTerminal(uint32_t nowMs) {
  _frame.fillRect(0, INFO_Y, SCREEN_W, INFO_H, TFT_BLACK);
  _frame.drawFastHLine(6, INFO_Y, SCREEN_W - 12, COL_TEXT_DIM);

  _frame.setTextColor(COL_TEXT_DIM, COL_BG);
  _frame.setTextDatum(TL_DATUM);
  _frame.drawString("TERMINAL", 8, INFO_Y + 2, 1);

  const int lineH = 9;
  const int maxVisible = (INFO_H - 16) / lineH;
  int y = INFO_Y + 12;
  int pos = 0;
  int lineIndex = 0;

  if (_typedChars <= 0) {
    bool showCursor = (_terminalLen > 0) && ((nowMs / 400) % 2 == 0);
    if (showCursor) {
      _frame.setTextColor(COL_TEXT, COL_BG);
      _frame.drawString("_", 8, y, 1);
    }
    return;
  }

  int activeLine = 0;
  for (int i = 0; i < _typedChars && i < _terminalLen; i++) {
    if (_terminalBuf[i] == '\n') activeLine++;
  }
  int scrollLine = 0;
  if (activeLine >= maxVisible) scrollLine = activeLine - maxVisible + 1;

  while (pos < _typedChars && lineIndex <= activeLine) {
    int lineStart = pos;
    while (pos < _typedChars && _terminalBuf[pos] != '\n') pos++;
    int lineLen = pos - lineStart;

    if (lineIndex >= scrollLine) {
      if (y >= INFO_Y + INFO_H - 4) break;

      char line[72];
      if (lineLen >= (int)sizeof(line)) lineLen = (int)sizeof(line) - 1;
      memcpy(line, _terminalBuf + lineStart, lineLen);
      line[lineLen] = '\0';

      bool atTypingEnd = (pos >= _typedChars);
      bool showCursor = atTypingEnd &&
                        ((_typedChars < _terminalLen) || ((nowMs / 400) % 2 == 0));
      if (showCursor && lineLen < (int)sizeof(line) - 2) {
        line[lineLen] = '_';
        line[lineLen + 1] = '\0';
      }

      _frame.setTextColor(COL_TEXT, COL_BG);
      _frame.drawString(line, 8, y, 1);
      y += lineH;
    }

    lineIndex++;
    if (pos < _typedChars && _terminalBuf[pos] == '\n') pos++;
  }
}

void ScannerUI::drawHeader(const PlanetData& p) {
  _frame.setTextColor(COL_TEXT_DIM, COL_BG);
  _frame.setTextDatum(TC_DATUM);
  _frame.drawString("PLANET WEATHER", SCREEN_W / 2, HEADER_Y, 1);
  _frame.setTextColor(COL_TEXT_HI, COL_BG);
  _frame.drawString(p.name, SCREEN_W / 2, HEADER_Y + 12, 2);
}

void ScannerUI::drawPlanetPicker(uint32_t nowMs) {
  // Uneven spacing + varied sizes (inner rocky worlds closer, giants farther out).
  static const int kPickerX[PlanetDatabase::COUNT] = {
    26, 42, 58, 72, 94, 118, 142, 162, 182, 200, 220
  };
  static const int kDotR[PlanetDatabase::COUNT] = {
    3, 4, 5, 4, 7, 8, 6, 6, 4, 5, 3
  };
  static const int kMiniSize[PlanetDatabase::COUNT] = {
    10, 11, 12, 11, 18, 17, 14, 14, 10, 12, 9
  };

  const int barY = PICKER_Y;
  const int cy = barY + PICKER_H / 2 + 2;

  _frame.fillRect(0, barY, SCREEN_W, PICKER_H, TFT_BLACK);
  _frame.drawFastHLine(6, barY, SCREEN_W - 12, COL_TEXT_DIM);

  _frame.setTextColor(COL_TEXT_DIM, COL_BG);
  _frame.setTextDatum(TL_DATUM);
  _frame.drawString("SELECT", 8, barY + 2, 1);

  const int sunX = PICKER_SUN_X;
  _frame.fillCircle(sunX, cy, 8, COL_SUN);
  _frame.fillCircle(sunX - 2, cy - 2, 2, COL_TEXT_HI);

  const int selected = _currentIndex;

  for (int i = 0; i < PlanetDatabase::COUNT; i++) {
    const int cx = kPickerX[i];
    const int dotR = kDotR[i];
    const int miniSz = kMiniSize[i];
    float bob = (i == selected) ? sinf(nowMs * 0.006f) * 3.0f : 0.0f;
    const int pcY = cy - (int)bob;

    if (i == selected) {
      _frame.drawCircle(cx, pcY, dotR + 3, COL_TEXT);
      PlanetRenderer::drawPlanetMini(_miniPlanet, PlanetDatabase::get(i),
                                     _rotation, _cloudRot, miniSz, nowMs);
      _miniPlanet.pushToSprite(&_frame, cx - miniSz / 2, pcY - miniSz / 2);
    } else {
      _frame.fillCircle(cx, pcY, dotR, PlanetDatabase::get(i).colorPrimary);
      _frame.drawCircle(cx, pcY, dotR, COL_TEXT_DIM);
    }
  }
}

void ScannerUI::drawScanEffect(uint32_t nowMs) {
  uint32_t elapsed = nowMs - _scanStartMs;
  float progress = (float)elapsed / (float)SCAN_ANIM_MS;

  int beamY = PLANET_CY - PLANET_SIZE / 2 + (int)(progress * PLANET_SIZE);
  _frame.drawFastHLine(PLANET_CX - PLANET_SIZE / 2, beamY, PLANET_SIZE, COL_TEXT);

  _frame.setTextColor(COL_TEXT, COL_BG);
  _frame.setTextDatum(TC_DATUM);
  _frame.drawString("SCANNING...", SCREEN_W / 2, PLANET_CY + PLANET_SIZE / 2 + 4, 2);
}

void ScannerUI::pushFrame() {
  _frame.pushSprite(0, 0);
}
