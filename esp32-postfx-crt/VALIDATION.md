# CRT / Arcade validation

Built with ESP-IDF 6.0.1 and flashed to the connected ESP32-S3 on 2026-09-23.
Jet dependency changes are saved locally in **a4f84d0**. The firmware was built
from those source changes before committing them (application version label
therefore records the preceding examples commit with a dirty suffix).

## Correctness

All six Release CTests passed with assertions enabled. The integrated scene test
checks two matched off/on poses, exact RGB565 scaling on physical odd rows,
unchanged even rows, buffer guards, repeated redraws, transitions and identical
serial/parallel output. Five low-level targets cover full-width, half-width,
packed fields, half-width packed fields and compile-time disabling. Each tests
both row parities, even/odd output heights, guarded buffers and intensity values
0, 1, 48, 112, 254 and 255 against an independent scaling reference.

The four-view native preview was visually inspected. The previously approved
Texture Lab also passed its native regression checks with CRT disabled.
No shared ESP32 scanout, pacing or raster-worker changes were needed.

## S3 measurements

A 34-second serial capture includes two full off/on cycles and the start of a
third. Across 32 reporting windows, cadence averaged
**59.99 fields/s**, ranging **59.97-59.99**.
No panics, unexpected resets or idle recovery yields were observed.

Excluding the first report following each switch (which can contain the previous
mode), settled windows from the first two cycles measured:

| Mode | Mean render time | Windows |
| --- | --- | --- |
| CRT off | 4.60 ms | 11 |
| CRT on, intensity 112 | 6.45 ms | 12 |

The difference is about **1.85 ms per field on average**.
Rendering includes scene preparation, both raster workers and the in-place CRT
pass. The pass runs only on the odd physical field; its odd-field cost is roughly
twice the mean, while even fields skip it. Both modes repeat the same motion,
although these are serial reporting windows rather than a locked-pose benchmark.

The on-screen HUD includes FPS, render MS, 12-16 accepted raster triangles depending
on pose, and triangles divided by actual render time. Scanout is about 16.30 ms.
The renderer processes compact 240x160 fields for a 480x320 LCD, at 60 fields/s;
each physical row updates at 30 Hz. Labels are composited after CRT and retain
full output resolution and brightness.

Startup memory: **84,627 bytes internal**, **8,385,552 bytes PSRAM** free.
The CRT effect allocates no additional image/row buffer. Original scene textures
occupy 70 KiB in flash, plus separate label bitmaps. Firmware is **0x71240 bytes**,
leaving 56% of the 1 MiB application partition free. The existing physical 16 MiB
versus configured 8 MiB flash warning is unchanged.

The example is running on the S3 for physical-display feedback. Visual approval
is pending. P4 defaults are included but were not tested on hardware.
