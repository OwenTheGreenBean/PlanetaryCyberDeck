#include "Planet.h"
#include "Config.h"
#include <Arduino.h>

static const PlanetData kPlanets[PlanetDatabase::COUNT] = {
  {"MERCURY", "-173/427 C", "0 km/h", "0 atm", "Solar Flare",
   "Exosphere (Na, K, O)", "Cratered regolith", "Sidereal lock; extreme thermal swing",
   "Scorched dayside, frozen nightside — no atmosphere shield",
   WeatherIcon::SolarFlare, RenderStyle::Mercury, 0x9CD3, 0x632C, 0x4228, 0.038f, 0.0f, false},
  {"VENUS", "462 C", "360 km/h", "92 atm", "Acid Rain",
   "CO2 / H2SO4 clouds", "Volcanic plains", "Runaway greenhouse world",
   "Sulfuric acid clouds; surface pressure 92 bar",
   WeatherIcon::AcidRain, RenderStyle::Venus, 0xFE60, 0xFD20, 0xFBE0, 0.028f, 0.045f, false},
  {"EARTH", "15 C", "45 km/h", "1 atm", "Cloud",
   "N2 / O2 / H2O", "Oceans & continents", "Active biosphere",
   "Liquid water oceans; hydrological cycle active",
   WeatherIcon::Cloud, RenderStyle::Earth, 0x051D, 0x2D06, 0xFFFF, 0.032f, 0.055f, false},
  {"MARS", "-63 C", "60 km/h", "0.006 atm", "Dust Storm",
   "CO2 (thin)", "Iron oxide dust", "Global dust storms possible",
   "Thin CO2 air; planet-wide dust events",
   WeatherIcon::DustStorm, RenderStyle::Mars, 0xC986, 0x8220, 0x4008, 0.034f, 0.060f, false},
  {"JUPITER", "-108 C", "620 km/h", "1000 atm", "Methane Storm",
   "H2 / He / CH4", "Gas giant", "Great Red Spot >300 years",
   "Great Red Spot anticyclone; 620 km/h band winds",
   WeatherIcon::MethaneStorm, RenderStyle::Jupiter, 0xC4A0, 0x8A22, 0xB2C0, 0.042f, 0.065f, false},
  {"SATURN", "-139 C", "1800 km/h", "1000 atm", "Lightning",
   "H2 / He", "Metallic hydrogen", "Iconic ring system",
   "Ring ice particles; lightning in atmosphere",
   WeatherIcon::Lightning, RenderStyle::Saturn, 0xE6D5, 0xC4A0, 0x9CD3, 0.040f, 0.058f, true},
  {"URANUS", "-195 C", "900 km/h", "1200 atm", "Ice Storm",
   "H2 / He / CH4", "Icy mantle", "98 deg axial tilt",
   "Cyan methane haze; extreme seasonal tilt",
   WeatherIcon::IceStorm, RenderStyle::Uranus, 0x4DFF, 0x3D9E, 0x6BF9, 0.030f, 0.040f, false},
  {"NEPTUNE", "-201 C", "2100 km/h", "1200 atm", "Cloud",
   "H2 / He / CH4", "Supersonic winds", "Strongest winds known",
   "2100 km/h jet streams; Great Dark Spot",
   WeatherIcon::Cloud, RenderStyle::Neptune, 0x0015, 0x0410, 0xFFFF, 0.032f, 0.070f, false},
  {"EUROPA", "-160 C", "0 km/h", "0 atm", "Ice Storm",
   "O2 (trace)", "Ice shell", "Subsurface ocean likely",
   "Fractured ice shell over liquid ocean",
   WeatherIcon::IceStorm, RenderStyle::Europa, 0xC618, 0x9CD3, 0x4228, 0.026f, 0.018f, false},
  {"TITAN", "-179 C", "0.3 m/s", "1.5 atm", "Methane Storm",
   "N2 / CH4 haze", "Hydrocarbon lakes", "Dense moon atmosphere",
   "Orange methane haze; hydrocarbon lakes",
   WeatherIcon::MethaneStorm, RenderStyle::Titan, 0xE4A0, 0xB2A0, 0x8220, 0.028f, 0.048f, false},
  {"PLUTO", "-229 C", "0 km/h", "1.0 Pa", "Snow",
   "N2 / CH4 / CO", "Nitrogen ice plains", "Tombaugh Regio heart",
   "Nitrogen ice heart plain; seasonal frost",
   WeatherIcon::Snow, RenderStyle::Pluto, 0x632C, 0xC986, 0xFBC0, 0.024f, 0.030f, false}
};

#define HS(label, lon, lat) { label, lon, lat }

// Lon spaced around the globe so rotation reveals new labels.
static const WeatherHotspot kHotspots[PlanetDatabase::COUNT][HOTSPOTS_PER_PLANET] = {
  { // Mercury
    HS("Dayside", 0.2f, 0.15f), HS("Nightside", 3.0f, -0.1f), HS("Caloris", -1.4f, 0.4f),
    HS("Polar Ice", 0.9f, 1.05f), HS("Craters", -2.2f, -0.55f), HS("Terminator", 1.7f, 0.0f),
    HS("Hollows", -0.5f, -0.7f)
  },
  { // Venus
    HS("Acid Deck", 0.4f, 0.35f), HS("Volcano", -1.1f, -0.25f), HS("Super-Wind", 1.6f, 0.55f),
    HS("Maxwell", -2.4f, 0.7f), HS("Ishtar", 2.5f, 0.9f), HS("Cloud Top", -0.3f, -0.6f),
    HS("Nightside", 3.1f, 0.1f)
  },
  { // Earth
    HS("Clouds", 0.5f, 0.45f), HS("Pacific", -1.8f, 0.0f), HS("Storm", 1.2f, -0.45f),
    HS("Ice Cap", 0.1f, 1.1f), HS("Desert", -0.7f, -0.35f), HS("Aurora", 2.4f, 0.95f),
    HS("Ocean", 2.0f, -0.7f)
  },
  { // Mars
    HS("Dust", -0.6f, 0.35f), HS("Polar Cap", 0.3f, 1.05f), HS("Valles", 1.4f, -0.25f),
    HS("Olympus", -1.9f, 0.2f), HS("Tharsis", 2.3f, 0.15f), HS("Hellasp", -2.6f, -0.5f),
    HS("Dunes", 0.9f, -0.75f)
  },
  { // Jupiter
    HS("Red Spot", 1.2f, -0.35f), HS("Band N", -0.5f, 0.55f), HS("Band S", 0.4f, -0.65f),
    HS("Methane", 2.1f, 0.2f), HS("Jet", -1.7f, 0.1f), HS("Oval BA", -2.5f, -0.4f),
    HS("Aurora", 0.0f, 1.15f)
  },
  { // Saturn
    HS("Rings", 0.0f, 0.05f), HS("Lightning", -1.2f, 0.45f), HS("Hex Pole", 0.6f, 1.15f),
    HS("Storm", 1.8f, -0.3f), HS("Band", -2.3f, 0.25f), HS("Cassini", 2.6f, -0.55f),
    HS("Enceladus", -0.4f, -0.9f)
  },
  { // Uranus
    HS("Ice Storm", -0.7f, 0.3f), HS("Dark Spot", 1.0f, -0.4f), HS("Haze", 0.2f, 0.75f),
    HS("Ring Arc", 2.2f, 0.1f), HS("Methane", -1.8f, -0.55f), HS("Pole", -2.6f, 1.05f),
    HS("Cloud", 1.6f, 0.5f)
  },
  { // Neptune
    HS("Dark Spot", -1.1f, 0.2f), HS("Jet Stream", 0.7f, 0.5f), HS("Cloud Deck", 1.6f, -0.25f),
    HS("Scooter", -2.2f, -0.45f), HS("Methane", 2.5f, 0.35f), HS("Wind Max", 0.0f, -0.7f),
    HS("Aurora", -0.4f, 1.1f)
  },
  { // Europa
    HS("Chaos", 0.8f, 0.05f), HS("Ice Crack", -0.9f, 0.55f), HS("Ocean?", 0.2f, -0.7f),
    HS("Linea", 1.9f, 0.35f), HS("Pwyll", -1.8f, -0.3f), HS("Ridge", 2.6f, -0.5f),
    HS("Plume?", -2.5f, 0.8f)
  },
  { // Titan
    HS("Kraken", -0.4f, -0.55f), HS("Haze", 1.0f, 0.4f), HS("Dunes", 1.5f, 0.05f),
    HS("Xanadu", -1.6f, 0.2f), HS("Ligeia", 2.3f, 0.7f), HS("River", -2.4f, -0.35f),
    HS("Cloud", 0.3f, 0.9f)
  },
  { // Pluto
    HS("Heart", 0.5f, -0.3f), HS("Frost", -0.7f, 0.7f), HS("N2 Plain", 0.1f, 0.45f),
    HS("Cthulhu", -1.9f, -0.4f), HS("Tenzing", 1.7f, 0.55f), HS("Bladed", 2.5f, -0.6f),
    HS("Haze", -2.6f, 0.15f)
  }
};

static const char* kPageNames[] = {
  "OVERVIEW", "ATMOSPHERE", "WEATHER", "SURFACE", "NOTES"
};

static const char* kIconLabels[] = {
  "Cloud", "Lightning", "Snow", "Dust Storm", "Acid Rain",
  "Methane Storm", "Solar Flare", "Ice Storm", "Volcanic Ash"
};

const PlanetData& PlanetDatabase::get(int index) {
  return kPlanets[wrapIndex(index)];
}

const WeatherHotspot* PlanetDatabase::getHotspots(int index) {
  return kHotspots[wrapIndex(index)];
}

int PlanetDatabase::wrapIndex(int index) {
  int n = COUNT;
  while (index < 0) index += n;
  return index % n;
}

int PlanetDatabase::randomIndex(int exclude) {
  if (COUNT <= 1) return 0;
  int idx;
  do {
    idx = random(0, COUNT);
  } while (idx == exclude);
  return idx;
}

const char* PlanetDatabase::pageName(InfoPage page) {
  uint8_t i = static_cast<uint8_t>(page);
  if (i >= static_cast<uint8_t>(InfoPage::PAGE_COUNT)) return "???";
  return kPageNames[i];
}

const char* PlanetDatabase::iconLabel(WeatherIcon icon) {
  return kIconLabels[static_cast<uint8_t>(icon)];
}
