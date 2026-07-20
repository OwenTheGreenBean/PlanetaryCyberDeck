#pragma once

#include <stdint.h>

enum class WeatherIcon : uint8_t {
  Cloud, Lightning, Snow, DustStorm, AcidRain,
  MethaneStorm, SolarFlare, IceStorm, VolcanicAsh
};

enum class RenderStyle : uint8_t {
  Mercury, Venus, Earth, Mars, Jupiter, Saturn,
  Uranus, Neptune, Europa, Titan, Pluto
};

enum class InfoPage : uint8_t {
  Overview = 0, Atmosphere, Weather, Surface, Notes, PAGE_COUNT
};

/// Surface annotation — leader line points to lon/lat on the rotating sphere.
struct WeatherHotspot {
  const char* label;
  float lon;   // Radians
  float lat;
};

struct PlanetData {
  const char* name;
  const char* temperature;
  const char* windSpeed;
  const char* pressure;
  const char* stormType;
  const char* atmosphere;
  const char* surfaceDesc;
  const char* notes;
  const char* summary;       // One-line brief for bottom bar
  WeatherIcon icon;
  RenderStyle style;
  uint16_t colorPrimary;
  uint16_t colorSecondary;
  uint16_t colorAccent;
  float rotationSpeed;
  float cloudSpeed;
  bool hasRings;
};

class PlanetDatabase {
public:
  static constexpr int COUNT = 11;

  static const PlanetData& get(int index);
  static const WeatherHotspot* getHotspots(int index);
  static int wrapIndex(int index);
  static int randomIndex(int exclude = -1);
  static const char* pageName(InfoPage page);
  static const char* iconLabel(WeatherIcon icon);
};
