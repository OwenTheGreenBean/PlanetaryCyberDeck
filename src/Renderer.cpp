#include "Renderer.h"
#include "Config.h"
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

// =============================================================================
// Fast procedural noise — deterministic hash + smooth value noise.
// Used for craters, continents, cloud wisps, and storm cells.
// =============================================================================

static inline float hash2f(float x, float y) {
  int ix = (int)(x * 127.1f + y * 311.7f);
  ix = (ix << 13) ^ ix;
  return (1.0f - ((ix * (ix * ix * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

static float valueNoise(float x, float y) {
  float ix = floorf(x);
  float iy = floorf(y);
  float fx = x - ix;
  float fy = y - iy;
  float ux = fx * fx * (3.0f - 2.0f * fx);
  float uy = fy * fy * (3.0f - 2.0f * fy);
  float a = hash2f(ix, iy);
  float b = hash2f(ix + 1.0f, iy);
  float c = hash2f(ix, iy + 1.0f);
  float d = hash2f(ix + 1.0f, iy + 1.0f);
  return a + (b - a) * ux + (c - a) * uy + (a - b - c + d) * ux * uy;
}

static float fbm(float x, float y, int octaves) {
  float sum = 0.0f;
  float amp = 0.5f;
  float freq = 1.0f;
  for (int i = 0; i < octaves; i++) {
    sum += valueNoise(x * freq, y * freq) * amp;
    freq *= 2.0f;
    amp *= 0.5f;
  }
  return sum;
}

// --- RGB565 helpers ---

static inline uint8_t rgb565_r(uint16_t c) { return (c >> 11) & 0x1F; }
static inline uint8_t rgb565_g(uint16_t c) { return (c >> 5) & 0x3F; }
static inline uint8_t rgb565_b(uint16_t c) { return c & 0x1F; }

static inline uint16_t make565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F);
}

static uint16_t lerp565(uint16_t a, uint16_t b, float t) {
  if (t <= 0.0f) return a;
  if (t >= 1.0f) return b;
  uint8_t r = rgb565_r(a) + (int)((rgb565_r(b) - rgb565_r(a)) * t);
  uint8_t g = rgb565_g(a) + (int)((rgb565_g(b) - rgb565_g(a)) * t);
  uint8_t b8 = rgb565_b(a) + (int)((rgb565_b(b) - rgb565_b(a)) * t);
  return make565(r, g, b8);
}

static uint16_t shade565(uint16_t c, float mul) {
  if (mul < 0.0f) mul = 0.0f;
  if (mul > 1.5f) mul = 1.5f;
  return make565(
    (uint8_t)(rgb565_r(c) * mul),
    (uint8_t)(rgb565_g(c) * mul),
    (uint8_t)(rgb565_b(c) * mul));
}

// =============================================================================
// Per-style surface shaders — each returns base albedo at (lon, lat).
// lon/lat in radians; style-specific features use fbm at mapped UV.
// =============================================================================

static uint16_t shadeMercury(float lon, float lat, const PlanetData& p) {
  float n = fbm(cosf(lon) * 3.0f + 2.0f, sinf(lat) * 3.0f + 1.0f, 4);
  float craters = fbm(cosf(lon) * 12.0f, sinf(lat) * 12.0f, 3);
  float craterMask = (craters > 0.62f) ? 0.75f : 1.0f;
  uint16_t base = lerp565(p.colorSecondary, p.colorPrimary, n);
  return shade565(base, craterMask);
}

static uint16_t shadeVenus(float lon, float lat, float cloudRot, const PlanetData& p) {
  float u = lon + cloudRot;
  float swirl = fbm(cosf(u) * 4.0f, sinf(lat) * 2.0f + cloudRot, 4);
  float band = sinf(lat * 6.0f + swirl * 2.0f) * 0.5f + 0.5f;
  return lerp565(p.colorSecondary, p.colorPrimary, band * 0.6f + swirl * 0.4f);
}

static uint16_t shadeEarth(float lon, float lat, float cloudRot, const PlanetData& p) {
  float u = cosf(lon) * 2.5f;
  float v = sinf(lat) * 2.5f;
  float land = fbm(u + 0.5f, v + 0.3f, 5);
  uint16_t surface = (land > 0.52f) ? p.colorSecondary : p.colorPrimary;
  // Polar ice caps
  if (fabsf(lat) > 1.2f) surface = lerp565(surface, 0xFFFF, (fabsf(lat) - 1.2f) * 2.0f);
  // Cloud layer (semi-transparent white wisps)
  float clouds = fbm(cosf(lon + cloudRot) * 6.0f, sinf(lat) * 4.0f + cloudRot, 3);
  if (clouds > 0.55f) {
    float ct = (clouds - 0.55f) * 2.2f;
    if (ct > 1.0f) ct = 1.0f;
    surface = lerp565(surface, p.colorAccent, ct * 0.85f);
  }
  return surface;
}

static uint16_t shadeMars(float lon, float lat, float cloudRot, const PlanetData& p) {
  float u = cosf(lon) * 3.0f;
  float v = sinf(lat) * 3.0f;
  float terrain = fbm(u, v, 4);
  uint16_t c = lerp565(p.colorAccent, p.colorPrimary, terrain);
  // Dark volcanic maria
  if (fbm(u * 1.5f + 3.0f, v * 1.5f, 3) > 0.65f)
    c = lerp565(c, p.colorSecondary, 0.55f);
  // Dust haze
  float dust = fbm(cosf(lon + cloudRot) * 5.0f, sinf(lat) * 3.0f, 2);
  if (dust > 0.6f) c = lerp565(c, 0xFD20, (dust - 0.6f) * 0.5f);
  return c;
}

static uint16_t shadeJupiter(float lon, float lat, float cloudRot, const PlanetData& p) {
  float u = lon + cloudRot;
  // Horizontal bands — latitude-driven
  float band = sinf(lat * 10.0f + sinf(u * 2.0f) * 0.3f) * 0.5f + 0.5f;
  uint16_t c = lerp565(p.colorSecondary, p.colorPrimary, band);
  // Turbulent streaks
  c = lerp565(c, p.colorAccent, fbm(cosf(u) * 8.0f, sinf(lat) * 6.0f, 2) * 0.25f);
  // Great Red Spot — fixed longitude pocket
  float spotLon = 1.2f;
  float dLon = u - spotLon;
  while (dLon > PI) dLon -= 2.0f * PI;
  while (dLon < -PI) dLon += 2.0f * PI;
  float spotDist = sqrtf(dLon * dLon * 4.0f + (lat - (-0.35f)) * (lat - (-0.35f)));
  if (spotDist < 0.35f) {
    float t = 1.0f - spotDist / 0.35f;
    c = lerp565(c, 0xC800, t * 0.9f);
  }
  return c;
}

static uint16_t shadeSaturn(float lon, float lat, float cloudRot, const PlanetData& p) {
  float band = sinf(lat * 8.0f + cloudRot * 0.5f) * 0.5f + 0.5f;
  return lerp565(p.colorSecondary, p.colorPrimary, band * 0.7f + fbm(cosf(lon) * 4.0f, sinf(lat) * 3.0f, 2) * 0.3f);
}

static uint16_t shadeUranus(float lon, float lat, float cloudRot, const PlanetData& p) {
  float u = lon + cloudRot;
  float haze = fbm(cosf(u) * 4.0f, sinf(lat) * 3.0f, 3);
  float band = sinf(lat * 6.0f + haze * 2.0f) * 0.5f + 0.5f;
  return lerp565(p.colorSecondary, p.colorPrimary, band * 0.55f + haze * 0.45f);
}

static uint16_t shadeNeptune(float lon, float lat, float cloudRot, const PlanetData& p) {
  float u = lon + cloudRot;
  uint16_t c = lerp565(p.colorSecondary, p.colorPrimary, fbm(cosf(u) * 3.0f, sinf(lat) * 3.0f, 3));
  // White storm cells
  float storm = fbm(cosf(u) * 10.0f + 5.0f, sinf(lat) * 8.0f, 2);
  if (storm > 0.68f) c = lerp565(c, p.colorAccent, (storm - 0.68f) * 3.0f);
  return c;
}

static uint16_t shadeEuropa(float lon, float lat, const PlanetData& p) {
  float u = cosf(lon) * 4.0f;
  float v = sinf(lat) * 4.0f;
  float ice = fbm(u, v, 3);
  uint16_t c = lerp565(p.colorSecondary, p.colorPrimary, ice);
  // Fracture lines (dark cracks)
  float crack = fabsf(sinf(u * 5.0f + fbm(u, v, 2) * 3.0f));
  if (crack < 0.08f) c = lerp565(c, p.colorAccent, 0.7f);
  return c;
}

static uint16_t shadeTitan(float lon, float lat, float cloudRot, const PlanetData& p) {
  float haze = fbm(cosf(lon + cloudRot) * 3.0f, sinf(lat) * 2.5f + cloudRot, 4);
  float band = sinf(lat * 4.0f + haze) * 0.5f + 0.5f;
  return lerp565(p.colorSecondary, p.colorPrimary, band * 0.5f + haze * 0.5f);
}

static uint16_t shadePluto(float lon, float lat, const PlanetData& p) {
  float u = cosf(lon) * 3.5f;
  float v = sinf(lat) * 3.5f;
  float frost = fbm(u + 1.0f, v, 4);
  uint16_t c = lerp565(p.colorPrimary, p.colorSecondary, frost);
  // Heart-shaped nitrogen plain (Sputnik Planitia approximation)
  float hx = cosf(lon + 0.5f) * 1.8f;
  float hy = sinf(lat) * 1.8f - 0.3f;
  float heart = hx * hx + hy * hy;
  if (heart < 0.9f) c = lerp565(c, p.colorAccent, 0.65f);
  return c;
}

static uint16_t sampleSurface(RenderStyle style, float lon, float lat,
                              float cloudRot, const PlanetData& p) {
  switch (style) {
    case RenderStyle::Mercury: return shadeMercury(lon, lat, p);
    case RenderStyle::Venus:   return shadeVenus(lon, lat, cloudRot, p);
    case RenderStyle::Earth:   return shadeEarth(lon, lat, cloudRot, p);
    case RenderStyle::Mars:    return shadeMars(lon, lat, cloudRot, p);
    case RenderStyle::Jupiter: return shadeJupiter(lon, lat, cloudRot, p);
    case RenderStyle::Saturn:  return shadeSaturn(lon, lat, cloudRot, p);
    case RenderStyle::Uranus:  return shadeUranus(lon, lat, cloudRot, p);
    case RenderStyle::Neptune: return shadeNeptune(lon, lat, cloudRot, p);
    case RenderStyle::Europa:  return shadeEuropa(lon, lat, p);
    case RenderStyle::Titan:   return shadeTitan(lon, lat, cloudRot, p);
    case RenderStyle::Pluto:   return shadePluto(lon, lat, p);
    default: return p.colorPrimary;
  }
}

// Storm-cell intensity at a point on the sphere (0..1), animated over time.
static float stormGlow(float lon, float lat, float cloudRot, RenderStyle style,
                       uint32_t animMs) {
  float t = animMs * 0.0015f;
  float u = cosf(lon + cloudRot) * 6.0f + t;
  float v = sinf(lat) * 5.0f - t * 0.7f;
  float n = fbm(u, v, 3);
  float pulse = sinf(t * 4.0f + lon * 2.5f + lat) * 0.5f + 0.5f;

  float threshold = 0.58f;
  switch (style) {
    case RenderStyle::Jupiter:
    case RenderStyle::Neptune:
    case RenderStyle::Venus:
      threshold = 0.50f;
      break;
    case RenderStyle::Earth:
    case RenderStyle::Mars:
      threshold = 0.55f;
      break;
    default:
      break;
  }

  float s = (n - threshold) * 2.8f;
  if (s < 0.0f) s = 0.0f;
  if (s > 1.0f) s = 1.0f;
  return s * (0.45f + pulse * 0.55f);
}

// Core sphere renderer — works at any square sprite size.
static void drawPlanetSized(TFT_eSprite& sprite, const PlanetData& planet,
                            float rotation, float cloudRot, int size,
                            uint32_t animMs) {
  const int cx = size / 2;
  const int cy = size / 2;
  const int r = cx - 1;
  const float r2 = (float)(r * r);

  // Key light (upper-left) + soft fill from camera-right for depth.
  const float lx = -0.62f, ly = -0.38f, lz = 0.70f;
  const float lx2 = 0.35f, ly2 = -0.15f, lz2 = 0.55f;

  sprite.fillSprite(TFT_BLACK);

  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      float dx = (float)(x - cx);
      float dy = (float)(y - cy);
      float d2 = dx * dx + dy * dy;
      if (d2 > r2) continue;

      float z = sqrtf(r2 - d2);
      float nx = dx / r;
      float ny = dy / r;
      float nz = z / r;

      float lon = atan2f(nx, nz) + rotation;
      float lat = asinf(ny);

      uint16_t albedo = sampleSurface(planet.style, lon, lat, cloudRot, planet);

      // Fine surface detail so rotation reads clearly on every planet type.
      float detail = fbm(cosf(lon) * 16.0f, sinf(lat) * 12.0f, 2);
      albedo = lerp565(albedo, shade565(albedo, detail > 0.5f ? 1.18f : 0.82f),
                       fabsf(detail - 0.5f) * 0.55f);

      // Dual-light Lambert shading for stronger 3D read.
      float ndl1 = nx * lx + ny * ly + nz * lz;
      float ndl2 = nx * lx2 + ny * ly2 + nz * lz2;
      if (ndl1 < 0.0f) ndl1 = 0.0f;
      if (ndl2 < 0.0f) ndl2 = 0.0f;
      float light = 0.12f + ndl1 * 0.72f + ndl2 * 0.18f;

      // Terminator — deep shadow on nightside.
      if (ndl1 < 0.15f) light *= 0.35f + ndl1 * 2.0f;

      // Specular highlight.
      float spec = nx * lx + ny * ly + nz * lz;
      if (spec > 0.88f) light += (spec - 0.88f) * 10.0f;

      // Atmospheric limb.
      float rim = 1.0f - nz;
      rim = rim * rim * 0.35f;

      uint16_t col = shade565(albedo, light);
      if (rim > 0.01f) col = lerp565(col, COL_TEXT_DIM, rim * 0.6f);

      // Animated storm emissive hotspots.
      float storm = stormGlow(lon, lat, cloudRot, planet.style, animMs);
      if (storm > 0.05f) {
        uint16_t hot = lerp565(albedo, COL_TEXT_HI, storm * 0.85f);
        hot = lerp565(hot, 0xFFFF, storm * 0.35f);
        col = lerp565(col, hot, storm * 0.75f);
      }

      sprite.drawPixel(x, y, col);
    }
  }

  if (planet.hasRings) {
    const int ringCy = cy + r / 5;
    for (int x = 2; x < size - 2; x++) {
      float t = (float)(x - cx) / (float)r;
      if (fabsf(t) > 1.45f) continue;
      int yOff = (int)(sinf(t * 2.5f) * 1.5f);
      int ry = ringCy + yOff;
      uint16_t backCol = lerp565(0x9CD3, 0x632C, fabsf(t) * 0.55f);
      uint16_t frontCol = lerp565(0xC618, 0x738E, fabsf(t) * 0.45f);
      if (ry < cy) sprite.drawPixel(x, ry, backCol);
      if (ry >= cy - 2) sprite.drawPixel(x, ry, frontCol);
    }
  }
}

void PlanetRenderer::drawPlanet(TFT_eSprite& sprite, const PlanetData& planet,
                                float rotation, float cloudRot, uint32_t animMs) {
  drawPlanetSized(sprite, planet, rotation, cloudRot, PLANET_SIZE, animMs);
}

void PlanetRenderer::drawPlanetMini(TFT_eSprite& sprite, const PlanetData& planet,
                                    float rotation, float cloudRot, int size,
                                    uint32_t animMs) {
  drawPlanetSized(sprite, planet, rotation, cloudRot, size, animMs);
}

void PlanetRenderer::blendPlanets(TFT_eSprite& out, TFT_eSprite& a, TFT_eSprite& b,
                                  float alpha, int size) {
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      uint16_t ca = a.readPixel(x, y);
      uint16_t cb = b.readPixel(x, y);
      if (ca == TFT_BLACK && alpha < 0.5f) {
        out.drawPixel(x, y, cb);
      } else if (cb == TFT_BLACK && alpha >= 0.5f) {
        out.drawPixel(x, y, ca);
      } else {
        out.drawPixel(x, y, lerp565(ca, cb, alpha));
      }
    }
  }
}

bool PlanetRenderer::projectPoint(float lon, float lat, float rotation,
                                  int pcx, int pcy, int pr, int* sx, int* sy) {
  float lr = lon + rotation;
  float nx = cosf(lat) * sinf(lr);
  float ny = sinf(lat);
  float nz = cosf(lat) * cosf(lr);
  if (nz < 0.08f) return false;
  *sx = pcx + (int)(nx * pr);
  *sy = pcy + (int)(ny * pr);
  return true;
}

void PlanetRenderer::drawLitSphere(TFT_eSprite& target, int cx, int cy, int radius,
                                   uint16_t color, bool satellite) {
  if (satellite) {
    target.fillRect(cx - radius, cy - 1, radius * 2, 3, color);
    target.drawRect(cx - radius - 2, cy - 3, radius * 2 + 4, 7, COL_TEXT_DIM);
    target.drawFastHLine(cx - radius - 4, cy, radius, COL_TEXT_DIM);
    target.drawFastHLine(cx + 1, cy, radius, COL_TEXT_DIM);
    return;
  }

  const float r2 = (float)(radius * radius);
  for (int dy = -radius; dy <= radius; dy++) {
    for (int dx = -radius; dx <= radius; dx++) {
      float d2 = (float)(dx * dx + dy * dy);
      if (d2 > r2) continue;
      float z = sqrtf(r2 - d2) / radius;
      float light = 0.25f + 0.75f * ((-dx - dy) * 0.04f + z * 0.5f);
      if (light > 1.2f) light = 1.2f;
      target.drawPixel(cx + dx, cy + dy, shade565(color, light));
    }
  }
}

// =============================================================================
// WeatherIconRenderer — procedural animated glyphs
// =============================================================================

void WeatherIconRenderer::draw(TFT_eSprite& spr, WeatherIcon icon, int x, int y,
                               int size, uint16_t color, uint32_t phase) {
  int cx = x + size / 2;
  int cy = y + size / 2;
  float t = (phase % 1000) / 1000.0f;

  switch (icon) {
    case WeatherIcon::Cloud: {
      spr.fillCircle(cx - 4, cy, 5, color);
      spr.fillCircle(cx + 3, cy - 1, 6, color);
      spr.fillCircle(cx + 8, cy + 1, 4, color);
      spr.fillRect(cx - 8, cy, 18, 5, color);
      break;
    }
    case WeatherIcon::Lightning: {
      spr.fillTriangle(cx - 2, cy - 8, cx + 4, cy - 8, cx - 1, cy, color);
      spr.fillTriangle(cx - 4, cy, cx + 2, cy, cx + 1, cy + 9, color);
      break;
    }
    case WeatherIcon::Snow: {
      for (int i = 0; i < 6; i++) {
        float a = i * PI / 3.0f + t * PI * 2.0f;
        spr.drawLine(cx, cy,
                     cx + (int)(cosf(a) * 7), cy + (int)(sinf(a) * 7), color);
      }
      break;
    }
    case WeatherIcon::DustStorm: {
      for (int i = 0; i < 5; i++) {
        int px = cx - 8 + (int)((i * 4 + phase / 30) % 16);
        int py = cy - 4 + (i % 3) * 3;
        spr.fillCircle(px, py, 2, color);
      }
      break;
    }
    case WeatherIcon::AcidRain: {
      spr.fillCircle(cx, cy - 5, 5, color);
      for (int i = 0; i < 3; i++) {
        int rx = cx - 4 + i * 4;
        int ry = cy + (int)(t * 10) % 8;
        spr.drawLine(rx, cy, rx - 1, cy + 6 + ry % 4, color);
      }
      break;
    }
    case WeatherIcon::MethaneStorm: {
      spr.drawCircle(cx, cy, 7, color);
      for (int i = 0; i < 8; i++) {
        float a = i * PI / 4.0f + t * 4.0f;
        spr.drawPixel(cx + (int)(cosf(a) * 9), cy + (int)(sinf(a) * 9), color);
      }
      break;
    }
    case WeatherIcon::SolarFlare: {
      spr.fillCircle(cx, cy, 4, COL_TEXT_HI);
      for (int i = 0; i < 8; i++) {
        float a = i * PI / 4.0f;
        int len = 6 + (int)(sinf(t * PI * 2 + i) * 3);
        spr.drawLine(cx, cy,
                     cx + (int)(cosf(a) * len), cy + (int)(sinf(a) * len), color);
      }
      break;
    }
    case WeatherIcon::IceStorm: {
      spr.drawCircle(cx, cy, 6, color);
      for (int i = 0; i < 4; i++) {
        float a = i * PI / 2.0f;
        spr.drawLine(cx, cy,
                     cx + (int)(cosf(a) * 8), cy + (int)(sinf(a) * 8), color);
      }
      break;
    }
    case WeatherIcon::VolcanicAsh: {
      spr.fillTriangle(cx, cy + 6, cx - 7, cy + 2, cx + 7, cy + 2, color);
      for (int i = 0; i < 4; i++)
        spr.fillCircle(cx - 4 + i * 3, cy - 2 - (phase / 40 + i) % 5, 2, COL_TEXT_HI);
      break;
    }
  }
}
