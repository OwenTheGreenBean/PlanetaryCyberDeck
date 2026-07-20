#include "UI.h"
#include "Renderer.h"
#include "Audio.h"
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
  _iconSprite.setColorDepth(16);
  _miniPlanet.setColorDepth(16);
  _frame.setColorDepth(16);

  _planetA.createSprite(PLANET_SIZE, PLANET_SIZE);
  _iconSprite.createSprite(24, 24);
  _miniPlanet.createSprite(MINI_PLANET_SIZE, MINI_PLANET_SIZE);
  _frame.createSprite(SCREEN_W, SCREEN_H);

  for (int i = 0; i < STAR_COUNT; i++) {
    uint32_t h = 1103515245u * (uint32_t)(i + 1) + 12345u;
    _starX[i] = h % SCREEN_W;
    _starY[i] = (uint8_t)((h >> 8) % (INFO_Y - 28));
    if (_starY[i] < HEADER_Y + 14) _starY[i] = HEADER_Y + 14;
  }

  _currentIndex = 0;  // Start on Mercury; buttons step from here
  generateOrbiters(_currentIndex);
  refreshTerminal();
  _lastFrameMs = millis();
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
}

void ScannerUI::wrapTerminalLines() {
  _wrappedCount = 0;
  const char* p = _terminalBuf;

  while (*p && _wrappedCount < MAX_TERMINAL_LINES) {
    while (*p == '\n') p++;
    if (!*p) break;

    const char* lineEnd = strchr(p, '\n');
    int lineLen = lineEnd ? (int)(lineEnd - p) : (int)strlen(p);

    int offset = 0;
    while (offset < lineLen && _wrappedCount < MAX_TERMINAL_LINES) {
      int chunk = lineLen - offset;
      if (chunk > TERMINAL_WRAP_CHARS) {
        chunk = TERMINAL_WRAP_CHARS;
        int breakAt = chunk;
        while (breakAt > 0 && p[offset + breakAt] != ' ') breakAt--;
        if (breakAt > 8) chunk = breakAt;
      }

      char* dst = _wrappedLines[_wrappedCount];
      memcpy(dst, p + offset, chunk);
      dst[chunk] = '\0';
      _wrappedCount++;

      offset += chunk;
      while (offset < lineLen && p[offset] == ' ') offset++;
    }

    p = lineEnd ? lineEnd + 1 : p + lineLen;
  }
}

void ScannerUI::refreshTerminal() {
  buildPlanetTerminal(PlanetDatabase::get(_currentIndex));
  wrapTerminalLines();
}

const char* ScannerUI::fieldLabel(WeatherField field) const {
  switch (field) {
    case WeatherField::Temp:        return "TEMP";
    case WeatherField::Wind:        return "WIND";
    case WeatherField::Pressure:    return "PRESS";
    case WeatherField::Storm:       return "STORM";
    case WeatherField::Atmosphere:  return "ATM";
    case WeatherField::Surface:     return "SURF";
    case WeatherField::Log:         return "LOG";
    default:                        return "???";
  }
}

bool ScannerUI::lineMatchesField(const char* line, WeatherField field) const {
  const char* key = fieldLabel(field);
  // Match "> KEY:" so wrapped continuation lines stay dim unless they start with key.
  char needle[16];
  snprintf(needle, sizeof(needle), "> %s:", key);
  return strstr(line, needle) != nullptr;
}

int ScannerUI::terminalScrollForHighlight() const {
  const int textH = INFO_H - 14;
  const int maxVisible = textH / TERMINAL_LINE_H;
  if (_wrappedCount <= maxVisible) return 0;

  int highlightLine = 0;
  for (int i = 0; i < _wrappedCount; i++) {
    if (lineMatchesField(_wrappedLines[i], _highlightField)) {
      highlightLine = i;
      break;
    }
  }

  // Keep highlighted line near the top of the visible window.
  int scroll = highlightLine;
  if (scroll > _wrappedCount - maxVisible) scroll = _wrappedCount - maxVisible;
  if (scroll < 0) scroll = 0;
  return scroll;
}

uint16_t ScannerUI::terminalLineColor(const char* line, uint32_t nowMs) const {
  if (lineMatchesField(line, _highlightField)) {
    // Flash the selected weather field.
    return ((nowMs / 280) % 2 == 0) ? COL_HL : COL_TEXT_HI;
  }

  if (strstr(line, "STATUS:") != nullptr)
    return ((nowMs / 250) % 2 == 0) ? COL_TEXT_HI : COL_TEXT_DIM;

  return COL_TEXT_DIM;
}

void ScannerUI::setPlanetIndex(int index, bool playSound) {
  index = PlanetDatabase::wrapIndex(index);
  if (index == _currentIndex) return;

  _currentIndex = index;
  generateOrbiters(index);
  _lastPlanetRenderMs = 0;
  refreshTerminal();

  if (playSound) Audio::get().playPlanetSwitch();
}

void ScannerUI::update(uint32_t nowMs, InputHandler& input) {
  if (nowMs - _lastFrameMs < FRAME_MS) return;
  uint32_t frameStart = micros();
  _lastFrameMs = nowMs;

  // Buttons step planets.
  ButtonEvent btn = input.pollButton();
  if (btn == ButtonEvent::PrevPlanet) {
    setPlanetIndex(_currentIndex - 1);
  } else if (btn == ButtonEvent::NextPlanet) {
    setPlanetIndex(_currentIndex + 1);
  }

  // Pot 1 dials absolute planet rotation (full turn across pot range).
  _rotation = input.rotationNorm() * (PI * 2.0f);

  // Pot 2 selects which weather field to highlight.
  _highlightField = input.weatherField();

  const PlanetData& cur = PlanetDatabase::get(_currentIndex);
  // Clouds keep drifting slowly for life; surface rotation is pot-driven.
  _cloudRot += cur.cloudSpeed * ROTATION_SCALE;

  _frame.fillSprite(TFT_BLACK);
  renderScene(nowMs);
  drawBottomPanel(nowMs);
  pushFrame();

  static uint32_t s_lastPrint = 0;
  static uint32_t s_frames = 0;
  static uint32_t s_usSum = 0;
  s_frames++;
  s_usSum += micros() - frameStart;
  if (nowMs - s_lastPrint >= 1000) {
    float avgMs = s_frames ? (s_usSum / (float)s_frames) / 1000.0f : 0;
    Serial.printf("[perf] SKIP_PLANET=%d  %.1f ms/frame  ~%.0f fps\n",
                  SKIP_PLANET_RENDER, avgMs,
                  avgMs > 0 ? 1000.0f / avgMs : 0.0f);
    s_lastPrint = nowMs;
    s_frames = 0;
    s_usSum = 0;
  }
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
  drawHeader(p, nowMs);

  const int px = PLANET_CX - PLANET_SIZE / 2;
  const int py = PLANET_CY - PLANET_SIZE / 2;

  drawOrbitPaths();
  drawOrbiters(nowMs, true);

#if SKIP_PLANET_RENDER
  // Perf test: flat colored circle instead of procedural sphere.
  const int pr = PLANET_SIZE / 2 - 2;
  _frame.fillCircle(PLANET_CX, PLANET_CY, pr, p.colorPrimary);
  (void)px;
  (void)py;
#else
  if (nowMs - _lastPlanetRenderMs >= PLANET_REDRAW_MS) {
    PlanetRenderer::drawPlanet(_planetA, p, _rotation, _cloudRot, nowMs);
    _lastPlanetRenderMs = nowMs;
  }
  _planetA.pushToSprite(&_frame, px, py);
#endif

  drawHotspotLabels(p, _rotation, _currentIndex);
  drawOrbiters(nowMs, false);
}

void ScannerUI::drawBatteryIcon(uint32_t nowMs) {
  const int bx = SCREEN_W - 30;
  const int by = 4;
  const bool low = (nowMs / 450) % 2 == 0;

  uint16_t frameCol = low ? COL_WARN : COL_TEXT_DIM;
  uint16_t fillCol = low ? COL_TEXT_HI : COL_TEXT;

  _frame.drawRoundRect(bx, by, 22, 11, 2, frameCol);
  _frame.fillRect(bx + 22, by + 3, 3, 5, frameCol);
  _frame.drawRoundRect(bx + 2, by + 2, 16, 7, 1, COL_TEXT_DIM);

  if (low) {
    _frame.fillRect(bx + 3, by + 3, 8, 5, fillCol);
    _frame.setTextColor(COL_WARN, COL_BG);
    _frame.setTextDatum(MR_DATUM);
    _frame.drawString("!", bx - 4, by + 5, 1);
  } else {
    _frame.fillRect(bx + 3, by + 3, 13, 5, fillCol);
  }
}

void ScannerUI::drawHeader(const PlanetData& p, uint32_t nowMs) {
  drawBatteryIcon(nowMs);

  const bool ping = (nowMs / 600) % 2 == 0;
  _frame.fillCircle(SCREEN_W - 44, 8, 3, ping ? COL_TEXT_HI : COL_TEXT_DIM);

  // Planet name only (no "PLANET WEATHER" subtitle).
  _frame.setTextColor(COL_TEXT, COL_BG);
  _frame.setTextDatum(TC_DATUM);
  _frame.drawString(p.name, SCREEN_W / 2, HEADER_Y, 2);
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

  // Pot 2 field cycles which hotspot is emphasized.
  int focusSpot = static_cast<int>(_highlightField) % HOTSPOTS_PER_PLANET;

  for (int i = 0; i < HOTSPOTS_PER_PLANET; i++) {
    int sx, sy;
    if (!PlanetRenderer::projectPoint(spots[i].lon, spots[i].lat, rotation,
                                     PLANET_CX, PLANET_CY, pr, &sx, &sy))
      continue;

    // Anchor labels off the limb: left side of disk → left, right → right.
    const bool left = sx < PLANET_CX;
    const bool upper = sy < PLANET_CY;
    // Stagger by index so nearby spots don't stack.
    const int staggerX = (i % 3) * 6;
    const int staggerY = ((i * 5) % 7) - 3;
    int lx = sx + (left ? -(42 + staggerX) : (42 + staggerX));
    int ly = sy + (upper ? -(12 + staggerY) : (12 + staggerY));

    if (lx < 2) lx = 2;
    if (lx > SCREEN_W - 4) lx = SCREEN_W - 4;
    if (ly < HEADER_Y + 18) ly = HEADER_Y + 18;
    if (ly > INFO_Y - 12) ly = INFO_Y - 12;

    const bool focus = (i == focusSpot);
    uint16_t lineCol = focus ? COL_HL : COL_TEXT_DIM;
    uint16_t textCol = focus ? COL_HL : COL_TEXT;

    _frame.drawLine(lx, ly, sx, sy, lineCol);
    _frame.fillCircle(sx, sy, focus ? 3 : 2, focus ? COL_HL : COL_TEXT_HI);

    _frame.setTextColor(textCol, COL_BG);
    _frame.setTextDatum(left ? TR_DATUM : TL_DATUM);
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

  const bool recOn = (nowMs / 500) % 2 == 0;
  _frame.fillCircle(18, INFO_Y + 6, 3, recOn ? COL_WARN : COL_TEXT_DIM);

  _frame.setTextColor(COL_TEXT_DIM, COL_BG);
  _frame.setTextDatum(TL_DATUM);
  _frame.drawString("TERMINAL", 26, INFO_Y + 2, 1);

  // Show which field Pot 2 is focusing.
  char focusTag[20];
  snprintf(focusTag, sizeof(focusTag), "[%s]", fieldLabel(_highlightField));
  _frame.setTextColor(COL_HL, COL_BG);
  _frame.setTextDatum(TR_DATUM);
  _frame.drawString(focusTag, SCREEN_W - 8, INFO_Y + 2, 1);

  const int textY = INFO_Y + 12;
  const int textH = INFO_H - 14;
  const int maxVisible = textH / TERMINAL_LINE_H;
  const int scroll = terminalScrollForHighlight();

  _frame.setTextDatum(TL_DATUM);
  int y = textY;
  for (int i = scroll; i < _wrappedCount && (i - scroll) < maxVisible; i++) {
    const bool hl = lineMatchesField(_wrappedLines[i], _highlightField);
    if (hl) {
      // Bar behind highlighted weather line.
      _frame.fillRect(4, y - 1, SCREEN_W - 8, TERMINAL_LINE_H, 0x2100);
    }
    _frame.setTextColor(terminalLineColor(_wrappedLines[i], nowMs), COL_BG);
    _frame.drawString(_wrappedLines[i], 8, y, 1);
    y += TERMINAL_LINE_H;
  }
}

void ScannerUI::drawPlanetPicker(uint32_t nowMs) {
  const int barY = PICKER_Y;
  const int cy = barY + PICKER_H / 2;

  _frame.fillRect(0, barY, SCREEN_W, PICKER_H, TFT_BLACK);
  _frame.drawFastHLine(6, barY, SCREEN_W - 12, COL_TEXT_DIM);

  _frame.setTextColor(COL_TEXT_DIM, COL_BG);
  _frame.setTextDatum(TL_DATUM);
  _frame.drawString("SELECT", 8, barY + 2, 1);

  const int sunX = PICKER_SUN_X;
  _frame.fillCircle(sunX, cy + 2, 7, COL_SUN);
  _frame.fillCircle(sunX - 2, cy, 2, COL_TEXT_HI);

  const int startX = 34;
  const int spacing = 18;
  const int selected = _currentIndex;

  for (int i = 0; i < PlanetDatabase::COUNT; i++) {
    const int cx = startX + i * spacing;
    float bob = (i == selected) ? sinf(nowMs * 0.006f) * 3.0f : 0.0f;
    const int pcY = cy + 2 - (int)bob;

    if (i == selected) {
      _frame.drawCircle(cx, pcY, 10, COL_TEXT);
#if SKIP_PLANET_RENDER
      _frame.fillCircle(cx, pcY, 7, PlanetDatabase::get(i).colorPrimary);
#else
      PlanetRenderer::drawPlanetMini(_miniPlanet, PlanetDatabase::get(i),
                                     _rotation, _cloudRot, MINI_PLANET_SIZE, nowMs);
      _miniPlanet.pushToSprite(&_frame, cx - MINI_PLANET_SIZE / 2,
                               pcY - MINI_PLANET_SIZE / 2);
#endif
    } else {
      _frame.fillCircle(cx, pcY, 5, PlanetDatabase::get(i).colorPrimary);
    }
  }
}

void ScannerUI::pushFrame() {
  _frame.pushSprite(0, 0);
}
