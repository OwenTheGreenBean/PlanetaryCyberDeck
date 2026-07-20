# Patch TFT_eSPI ESP32-C3 SPI register base — REG_SPI_BASE(SPI2_HOST) is 0 on C3.
# See: soc.h only maps index 2, but SPI2_HOST == 1.

Import("env")
import os
import glob

def patch_tft_espi_c3():
    libdeps = env.get("PROJECT_LIBDEPS_DIR", "")
    if not libdeps:
        return
    pattern = os.path.join(libdeps, "*", "TFT_eSPI", "Processors", "TFT_eSPI_ESP32_C3.h")
    for path in glob.glob(pattern):
        with open(path, "r") as f:
            content = f.read()
        old = """#if CONFIG_IDF_TARGET_ESP32C3
  // Fix ESP32C3 IDF bug for missing definition (VSPI/FSPI only tested at the moment)
  #ifndef REG_SPI_BASE
    #define REG_SPI_BASE(i) DR_REG_SPI2_BASE
  #endif"""
        new = """#if CONFIG_IDF_TARGET_ESP32C3
  // Fix ESP32-C3: soc REG_SPI_BASE only accepts index 2, SPI2_HOST is 1
  #ifdef REG_SPI_BASE
  #undef REG_SPI_BASE
  #endif
  #define REG_SPI_BASE(i) DR_REG_SPI2_BASE"""
        if old in content:
            content = content.replace(old, new)
            with open(path, "w") as f:
                f.write(content)
            print("Patched TFT_eSPI ESP32-C3 REG_SPI_BASE:", path)

patch_tft_espi_c3()
