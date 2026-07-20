#pragma once

#define TFT_SCK_PIN  4
#define TFT_MOSI_PIN 6
#define TFT_DC_PIN   7
#define TFT_RST_PIN  2
#define TFT_CS_PIN   10

// Buttons → GND (INPUT_PULLUP)
#define BTN_PREV_PIN  0   // Button 1 — previous planet
#define BTN_NEXT_PIN  1   // Button 2 — next planet

// Pots (center pin); outer pins to 3V3 / GND
#define POT_ROTATE_PIN  3 // Pot 1 — spin the planet
#define POT_FIELD_PIN   5 // Pot 2 — highlight weather fields

#define BUZZER_PIN  8     // Passive buzzer (+)

// Legacy aliases (older code / docs)
#define BTN_SCAN_PIN   BTN_PREV_PIN
#define BTN_MENU_PIN   BTN_NEXT_PIN
#define POT_PLANET_PIN POT_ROTATE_PIN
#define POT_PAGE_PIN   POT_FIELD_PIN

#define SCREEN_W 240
#define SCREEN_H 320

#define PLANET_SIZE 128
#define MINI_PLANET_SIZE 16
#define MOON_SPRITE_SIZE 14

#define TARGET_FPS           30
#define FRAME_MS             (1000 / TARGET_FPS)
#define SCAN_ANIM_MS         2500
#define ROTATION_SCALE       1.0f

#define STAR_COUNT           48

#define MAX_ORBIT_BODIES     5
#define HOTSPOTS_PER_PLANET  7
#define TERMINAL_BUF_SIZE    320
#define TERMINAL_WRAP_CHARS  36
#define MAX_TERMINAL_LINES   24
#define TERMINAL_LINE_H      9
#define PLANET_REDRAW_MS     90

// Set to 1 to skip procedural planet draws (perf test).
#define SKIP_PLANET_RENDER   1

/// Weather fields selectable with Pot 2 (highlight in terminal + hotspot).
#define WEATHER_FIELD_COUNT  7

#define COL_BG        0x0000
#define COL_BG2       0x0000
#define COL_TEXT      0x07E0
#define COL_TEXT_DIM  0x0320
#define COL_TEXT_HI   0x47E0
#define COL_WARN      0xFD20
#define COL_SUN       0x5FE0
#define COL_HL        0xFFE0  // Bright yellow highlight

#define COL_CREAM     COL_TEXT
#define COL_CYAN      COL_TEXT
#define COL_ORANGE    COL_TEXT_HI
#define COL_DIM       COL_TEXT_DIM
#define COL_GLOW      COL_TEXT_DIM

#define HEADER_Y      8
#define PLANET_CX     (SCREEN_W / 2)
#define PLANET_CY     102
#define INFO_Y        174
#define INFO_H        100
#define PICKER_Y      276
#define PICKER_H      44
#define PICKER_SUN_X  14
