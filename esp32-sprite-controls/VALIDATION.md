# Air Mail validation

Built with ESP-IDF 6.0.1 and flashed to the ESP32-S3 on COM6 on 2026-09-23.
Jet remains at fffbf63; no dependency or shared-runtime changes were required.
P4 defaults are included but were not hardware tested.

## Native verification

Release CTest passed with assertions enabled. The actual asymmetric plane asset
was checked under horizontal, vertical and combined flips. Eight rendered
composites match between serial and parallel rasterization, covering both field
parities, buffer guards and all four stages. Stage boundaries and a completely
black fade beneath still-visible captions are checked. The preview montage was
inspected for composition, direction changes, echoes and cinematic framing.

## S3 measurements

A 34-second serial capture includes the complete 28-second cycle and its
restart. Across 32 windows, cadence averaged 59.14 fields/s
(range 52.76-59.86). No panics or idle recovery
were observed. Accepted world triangles ranged 338-378.

| Stage | Mean fields/s | Mean render ms | Mean scanout ms |
| --- | --- | --- | --- |
| DIRECTION / FLIP X | 59.53 | 4.92 | 16.72 |
| INVERT / FLIP Y | 59.69 | 4.96 | 16.68 |
| ECHO / COMBINED ALPHA | 59.61 | 5.01 | 16.70 |
| CINEMA / BARS + FADE | 57.05 | 5.80 | 17.50 |

Stage figures exclude their first reporting window. They describe moving
scenes, not matched-pose microbenchmarks. Sprite compositing occurs during
scanout, so echoes and the full-screen fade affect scanout/cadence rather than
render MS or triangle count. The cinematic stage also includes periods of
zero fade and opaque black; its average is not a pure alpha-blending cost.

Startup free memory: 24,403 bytes internal, 8,371,984 bytes PSRAM.
Firmware size: 0x6fee0 bytes. The plane uses one immutable 4 KiB texture
shared by every echo; no additional image buffer is allocated.

The user approved Air Mail on the S3 and its demonstration of sprite effects
over a 3D scene.
