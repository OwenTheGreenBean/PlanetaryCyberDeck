#pragma once

#define TFT_SCK_PIN  4
#define TFT_MOSI_PIN 6
#define TFT_DC_PIN   7
#define TFT_RST_PIN  2
#define TFT_CS_PIN   10

#define BTN_SCAN_PIN  0
#define BTN_MENU_PIN  1
#define POT_PLANET_PIN 3
#define POT_PAGE_PIN   5

#define SCREEN_W 240
#define SCREEN_H 320

#define PLANET_SIZE 128
#define MINI_PLANET_SIZE 20
#define MOON_SPRITE_SIZE 14

#define TARGET_FPS           30
#define FRAME_MS             (1000 / TARGET_FPS)
#define AUTO_PLANET_MS       20000
#define SCAN_ANIM_MS         2500
#define ROTATION_SCALE       1.0f

#define STAR_COUNT           48

#define MAX_ORBIT_BODIES     5
#define HOTSPOTS_PER_PLANET  7
#define TERMINAL_BUF_SIZE    280
#define TYPE_CHAR_MS         10
#define TYPE_LINE_PAUSE_MS   20
#define PLANET_REDRAW_MS     90

#define COL_BG        0x0000
#define COL_BG2       0x0000
#define COL_TEXT      0x07E0
#define COL_TEXT_DIM  0x0320
#define COL_TEXT_HI   0x47E0
#define COL_SUN       0x5FE0

#define COL_CREAM     COL_TEXT
#define COL_CYAN      COL_TEXT
#define COL_ORANGE    COL_TEXT_HI
#define COL_DIM       COL_TEXT_DIM
#define COL_GLOW      COL_TEXT_DIM

#define HEADER_Y      22
#define PLANET_CX     (SCREEN_W / 2)
#define PLANET_CY     102
#define INFO_Y        174
#define INFO_H        100
#define PICKER_Y      276
#define PICKER_H      44
#define PICKER_SUN_X  14
