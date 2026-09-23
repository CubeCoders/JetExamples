# Validation — 23 September 2026

Both ESP32-S3 and ESP32-P4 firmware images build successfully with ESP-IDF
6.0.1 and the supplied defaults. The P4 has not been flashed as part of this
extraction.

The S3 image was flashed to the connected ESP32-S3 revision 0.2 at 240 MHz,
with 8 MB octal PSRAM. Flash contents passed esptool's verification.
An 18-second serial capture confirmed startup and three timing reports:

```
Queued scanout: DMA channel 0, 8 rows, interrupt status 0
Jet ready: 480x320, 60 fields/s, raster cores 1+0, internal free 111971, PSRAM free 8386156
Cadence 60.00 fields/s; render 0.30 ms, scanout 16.23 ms; idle recovery 0
Cadence 60.00 fields/s; render 0.29 ms, scanout 16.23 ms; idle recovery 0
Cadence 60.00 fields/s; render 0.30 ms, scanout 16.23 ms; idle recovery 0
```

No panic, DMA timeout or task-watchdog report occurred in that capture. The
module reports 16 MB physical flash; the generic example config deliberately
uses only 8 MB, which IDF reports at boot. Physical LCD appearance is left for
the operator to confirm; the host-generated cube image was visually inspected.

All three MSVC Release host checks pass with assertions enabled:

- Frame schedule: 60,000 deadlines, variable latency, overruns and restart.
- Cube: one object with 12 triangles, visible rotation over 120 fields and
  intact framebuffer guards.
- Sprite scanlines: 96,768 clipping/scaling/transform/blending cases, each in
  native and byte-swapped RGB565, compared with an independent reference.

Dependency review branches are `codex/esp32-template-support` in each submodule.
Jet has separate commits for the imported pending renderer optimisations
(`77dcff8`) and the public scanline compositor (`2c567e4`). LovyanGFX preserves
the existing P4 DMA/cache optimisation commit (`2aa581eb`). Publish these
dependencies to reachable remotes before publishing the examples repository.
