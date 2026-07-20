#pragma once

#include <TFT_eSPI.h>
#include "Planet.h"
#include "Config.h"

class PlanetRenderer {
public:
  static void drawPlanet(TFT_eSprite& sprite, const PlanetData& planet,
                         float rotation, float cloudRot, uint32_t animMs);

  static void drawPlanetMini(TFT_eSprite& sprite, const PlanetData& planet,
                             float rotation, float cloudRot, int size,
                             uint32_t animMs);

  static void blendPlanets(TFT_eSprite& out, TFT_eSprite& a, TFT_eSprite& b,
                           float alpha, int size);

  /// Project a lon/lat on the rotating sphere to screen coords (front hemisphere only).
  static bool projectPoint(float lon, float lat, float rotation,
                           int pcx, int pcy, int pr, int* sx, int* sy);

  /// Draw a small shaded 3D sphere (moon / satellite body).
  static void drawLitSphere(TFT_eSprite& target, int cx, int cy, int radius,
                            uint16_t color, bool satellite);
};

class WeatherIconRenderer {
public:
  static void draw(TFT_eSprite& sprite, WeatherIcon icon, int x, int y,
                   int size, uint16_t color, uint32_t animPhase);
};
