# Planet Weather Scanner — Setup Guide

## PlatformIO (recommended for this repo)

1. Open the project folder in VS Code / Cursor with PlatformIO.
2. Dependencies install automatically from `platformio.ini` (`TFT_eSPI`).
3. TFT pins and driver are configured in `include/User_Setup.h` (included via build flag).
4. Build & upload:
   ```bash
   pio run -t upload
   ```
5. Default environment: `dfrobot_beetle_esp32c3`. For a generic C3 dev board use `esp32-c3-devkitm-1`.

## Arduino IDE

1. Copy the `PlanetScanner/` folder into your Arduino sketches directory.
2. Install **TFT_eSPI** by Bodmer (Library Manager).
3. Configure TFT_eSPI — choose **one** method:
   - **Option A:** Copy `User_Setup.h` from the sketch folder into  
     `Arduino/libraries/TFT_eSPI/User_Setup.h` (backup the original first).
   - **Option B:** Edit `TFT_eSPI/User_Setup_Select.h` and comment all `#include` lines, then add:  
     `#include <User_Setup.h>` and place this project's `User_Setup.h` in your sketch folder (already included).
4. Board: **ESP32C3 Dev Module** (or your specific C3 board).
5. Open `PlanetScanner.ino` and upload.

## Wiring Summary

| Function | GPIO |
|----------|------|
| TFT SCK  | 4    |
| TFT MOSI | 6    |
| TFT DC   | 7    |
| TFT RST  | 2    |
| TFT CS   | 10   |
| Button SCAN | 0 |
| Button MENU | 1 |
| Pot Planet  | 3 |
| Pot Page    | 5 |

Buttons: `INPUT_PULLUP`, active LOW.  
Pots: outer pins to GND and 3.3V, wiper to GPIO.

## Controls

| Control | Action |
|---------|--------|
| Upper pot | Select planet (11 worlds) |
| Lower pot | Info page: Overview / Atmosphere / Weather / Surface / Notes |
| Button GPIO0 | SCAN — sweep animation |
| Button GPIO1 | MENU — system overlay |

Every **20 seconds** the scanner auto-transitions to a random planet with a cross-fade.

## Performance Notes

- Full-screen sprite compositing (~240×320) plus 88×88 planet sprites.
- Target **~30 FPS** via frame pacing in `Config.h` (`TARGET_FPS`).
- If you see flicker or low FPS, reduce `PLANET_SIZE` in `Config.h` (e.g. 72).

## Display Driver

The default configuration assumes a **2.0" ST7789** panel at **240×320** portrait.  
If colors are wrong or the image is offset, try `ILI9341_DRIVER` instead of `ST7789_DRIVER` in `User_Setup.h`.

## ESP32-C3 TFT_eSPI patch (required)

PlatformIO runs `extra_scripts/fix_tft_espi_c3.py` before each build. This fixes a known bug where `REG_SPI_BASE(SPI2_HOST)` resolves to `0` on ESP32-C3 and crashes in `tft.init()`.

**Quick display test** (color cycle + animation bar):

```bash
pio run -e screen_test -t upload
```

Serial should report `Calling tft.init()... OK` and `TFT 240x320`.

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| Guru Meditation on boot | Rebuild after `pio run -t clean`; ensure the C3 patch script ran |
| Blank white screen | Check wiring; try `ILI9341_DRIVER` |
| Wrong colors | Add `#define TFT_RGB_ORDER TFT_BGR` to `User_Setup.h` |
| Upside-down | Change `tft.setRotation(0)` to `1` or `3` in `UI.cpp` |
