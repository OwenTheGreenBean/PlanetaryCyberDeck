// =============================================================================
// TFT_eSPI User Setup — Planet Weather Scanner (ESP32-C3)
// =============================================================================
// PlatformIO loads this via build flag: -include include/User_Setup.h
//
// Arduino IDE: copy this file to your TFT_eSPI library folder, replacing or
// renaming the active setup, OR edit User_Setup_Select.h to include this file.
//
// IMPORTANT (ESP32-C3): Do NOT define USE_HSPI_PORT — the C3 has one SPI bus
// (FSPI). USE_HSPI_PORT leaves TFT_MISO at -1 and can crash in tft.init().
// =============================================================================

#define USER_SETUP_LOADED 1

#define ST7789_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_MOSI 6
#define TFT_SCLK 4
#define TFT_MISO -1   // Not connected — TFT_eSPI C3 path mirrors MOSI internally
#define TFT_CS   10
#define TFT_DC   7
#define TFT_RST  2

#define LOAD_GLCD  1
#define LOAD_FONT2 1
#define LOAD_FONT4 1
#define LOAD_FONT6 1
#define LOAD_FONT7 1
#define LOAD_FONT8 1
#define LOAD_GFXFF 1

#define SMOOTH_FONT 1

#define SPI_FREQUENCY       27000000
#define SPI_READ_FREQUENCY  20000000

// Do NOT enable USE_HSPI_PORT on ESP32-C3
