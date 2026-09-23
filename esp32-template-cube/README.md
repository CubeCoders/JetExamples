# ESP32 rotating cube template

Use ESP-IDF **6.0.x** (validated by compilation with 6.0.1), an ESP32-S3 or
ESP32-P4 with PSRAM, and a 320x480 ST7796 SPI LCD used in landscape at 480x320.
This is an SPI example, including on P4; it does not use the DSI panel path.

From this directory, in an ESP-IDF terminal:

```sh
# S3
idf.py -B build-s3 -DIDF_TARGET=esp32s3 -DSDKCONFIG=sdkconfig.s3 build
idf.py -B build-s3 -DIDF_TARGET=esp32s3 -DSDKCONFIG=sdkconfig.s3 -p PORT flash monitor

# Or P4, with separate build and configuration files
idf.py -B build-p4 -DIDF_TARGET=esp32p4 -DSDKCONFIG=sdkconfig.p4 build
idf.py -B build-p4 -DIDF_TARGET=esp32p4 -DSDKCONFIG=sdkconfig.p4 -p PORT flash monitor
```

Replace `PORT` with your serial device. In PowerShell, quote each `-D...` argument
if your shell splits the dot in the configuration filename. Defaults select
8 MB flash; S3 uses octal PSRAM at 80 MHz, and P4 uses PSRAM at 200 MHz.
Adapt flash/PSRAM settings with `menuconfig` for your module before flashing.

## Reference wiring

| Signal | S3 GPIO | P4 GPIO |
| --- | ---: | ---: |
| SCLK | 46 | 20 |
| MOSI | 3 | 5 |
| D/C | 8 | 23 |
| CS | 17 | 7 |
| Reset | 18 | 8 |
| Backlight | 9 | 21 |

No MISO or input buttons are required. Both use a dedicated SPI2 bus at 80 MHz;
S3 uses SPI mode 1 and P4 mode 0. Edit `../components/esp32_jet/Board.hpp` for
your wiring, bus rate or panel settings. These pins are reference-board wiring,
not universal ESP32 pin assignments. The S3 queue specifically owns SPI2's DMA
completion interrupt. Do not share that bus with other devices or display tasks.

## Your application

`main/main.cpp` starts the runtime with two callbacks from `main/Cube.hpp`:

- `init(Scene&)`: create the camera and scene objects with application lifetime.
- `update(float seconds)`: update scene state before rendering on core 1.

Keep `main/firmware/JetConfig.hpp` consistent across all components. The shared
CMake setup handles this. Field buffers are required by the overlapping runtime;
unsupported single-buffer/checkerboard configurations fail at compile time.
Half-width output can be disabled, at twice the framebuffer memory cost. Z-buffer
builds use single-core rasterization. Parallel water must sample the previous
field buffer, which the runtime supplies.

Sprites/materials are snapshotted before the next update; their texture pixels
remain borrowed. Keep those pixels alive and immutable during scanout. The
compositor uses a stable z-order sort once per field, outside the scanline loop.
Do not modify the scene from other tasks without synchronization.

The renderer and LCD initializer run on core 0; rendering runs on core 1.
Allocation or task creation failures stop initialization rather than starting
a frame loop with invalid buffers. Serial statistics are printed every 300 fields.


All examples include the shared FPS/TRIS/TRI/S overlay. FPS measures completed
render fields; TRI/S sums the culled/rasterized counts over the same sampling
window. See the repository README for exact counter definitions.
