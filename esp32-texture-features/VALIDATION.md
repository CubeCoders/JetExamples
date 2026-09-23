# Texture Lab validation

Validated on the connected ESP32-S3 on 2026-09-23, using ESP-IDF 6.0.1,
Jet e3ac6e8 and the shared 480x320 half-width alternating-field runtime.
The example uses painter rendering without a depth buffer.

## Native correctness and visual preview

Release CTest passed with assertions enabled. Checks cover negative/out-of-range
wrap, clamp and zero addressing; actual keyed holes in the rendered foliage;
palette animation with immutable source indices; identical LOD-off/on near
endpoints and different far endpoints; every stage transition and loop wrap;
buffer guards; and 48 serial/parallel parity and band-split comparisons.
An eight-view montage was inspected, including the LOD fading and flat states.
Native preview counters are placeholders, not performance measurements.

## Hardware

Built and flashed successfully on COM6. A 58-second serial capture includes all
seven stages and the return to WRAP. Across 56 reporting windows, cadence
averaged **60.00 fields/s**, ranging **59.98-60.00**.
No unexpected resets, panics or idle recovery yields were observed.

Representative settled render times (scene setup plus both raster workers,
excluding display pacing) were:

| Stage | Render time |
| --- | --- |
| Wrap | 3.04-3.06 ms |
| Clamp | 7.17-7.18 ms |
| Zero plus black key | 6.05-6.14 ms |
| Colour-key panels | 6.59-6.60 ms |
| Indexed palette cycling | 7.30-7.32 ms |
| LOD off, far part of sweep | about 2.42 ms |
| LOD on, far part of sweep | about 0.80 ms |

The LOD rows are one-second reporting windows near the far endpoint, not isolated
microbenchmarks. Both stages use the same distance sweep. The fade band still
samples and blends; it can cost more than LOD-off at the same size. The far solid
fill removes per-pixel texture lookup. Addressing paths also differ in which
renderer fast paths they can use, so these timings are specific to this scene
and build configuration. Switch-adjacent windows may include two modes.

The HUD shows FPS, render milliseconds, accepted raster triangles (16 normally,
18 for the two foliage cards), and triangles per actual render second. Hidden
triangles can still count; this is not a visible-pixel query.

Startup free memory: **83,235 bytes internal**, **8,385,396 bytes PSRAM**.
Texture pixels and palette total **39,040 bytes in flash**; label bitmaps are
additional. Firmware is **0x991b0 bytes**, leaving 40% of the 1 MiB application
partition free. The board's existing 16 MiB physical / 8 MiB configured flash
warning is unchanged.

The example is running on the S3 for display-level feedback. The complete effect cycle was subsequently
visually approved on the S3 before starting CRT / Arcade. P4 defaults are supplied but were not tested here.
