# Utah teapot validation — 2026-09-23

ESP32-S3 revision 0.2, 240 MHz, 8 MiB octal PSRAM at 80 MHz; 480×320 panel,
80 MHz SPI, eight-row DMA queue, half-width alternating field buffers.
Firmware built with ESP-IDF 6.0.1 and flashed to the connected S3.

## Timing

Early serial averages span 300 render fields; the final cycle reports every 60. Values vary with the animated pose;
these are observed runs, not a fixed-pose microbenchmark.

| Configuration | Field FPS | Render time | Notes |
| --- | ---: | ---: | --- |
| Depth-correct baseline | 15.34 | 65.14 ms | One raster core, double-precision square root |
| Exact float-assisted integer square root | 29.55–29.71 | about 33.6 ms | Same shading and geometry; host images unchanged |
| Two raster cores, redundant per-object sort removed | 50.65–50.97 | 19.54–19.65 ms | Original broad specular effect |
| First glossy material, faster rocking, 32 KiB internal scratch limit and all counters | 44.68–45.73 | 21.80–22.30 ms | Setup about 5.24 ms, raster 16.57–17.06 ms |

That first glossy run's scanout averages 16.28 ms, overlapping rendering. Sampled throughput
is approximately 27,462–30,065 unique rasterized triangles/second, with 618–732
triangles in the fields captured at reporting time. Counts vary through rotation.
No watchdog recovery yields, resets or allocation failures occurred in the
24-second capture. The teapot still contains all 1,560 source triangles.

The final red teapot cycles modes every three seconds. Settled windows from
the 24-second S3 capture (excluding windows spanning a mode transition):

| Mode | Field FPS | Render time | Sampled triangles/second |
| --- | ---: | ---: | ---: |
| Flat | 59.99–60.00 | 11.24–12.35 ms | about 33,000–38,000 |
| Gouraud | 59.70–60.00 | 13.91–16.43 ms | about 33,000–43,000 |
| Glossy Phong | 49.13–53.78 | 18.52–20.25 ms | about 29,600–33,100 |

The extra gloss optimisation skips exactly-zero highlights and reuses the
diffuse dot product. Front-to-back opaque buckets reject hidden pixels earlier.
Scanout remains about 16.30 ms and the capture has zero idle recovery yields.
These animated windows cover different poses; they show observed throughput,
not an isolated benchmark or a guaranteed frame rate.

The overlay's FPS means render **fields** per second. Each physical LCD row
updates on alternate fields. The user confirmed that the first glossy build
was stable and that its gloss and motion looked good. The final red colour,
brighter gradient, cycle and additional optimisations were subsequently approved on S3.

## Correctness

- Fixed two depth-addressing defects: pixel writes omitted the X offset;
  field clears used packed colour Y addressing against a full-height depth buffer.
- Depth tests pass for half-width fields, half-width full frames and full-width
  full frames, including crossing slopes, reversed draw order and guard memory.
- The square-root fast path passes 3,542,338 exact comparisons, including every
  squared normal magnitude, square boundaries and randomized 32/64-bit inputs.
  Four baseline host views are pixel-identical with the faster square root.
- Removing the redundant sort changes eight equal-depth tie pixels across
  a 614,400-pixel four-view baseline. Parallel band execution itself is exact.
- Forty concurrent raster cases compare colour, depth, guard values and unique
  triangle counts with serial output across poses, parities and band boundaries.
- Gloss tests compare integer exponents 1, 2, 8, 32, 128 and 255 against `pow`,
  check zero/back-facing cases and coloured light, and exhaust RGB565 saturation.
- The fast exponent-32 highlight path matches the generic integer calculation
  over all 32,769 Q15 cosine values at 16 strength/intensity combinations.
- Front-to-back sorting changes two equal-depth tie pixels across the 614,400-pixel
  red glossy montage. A separate test covers 24 submission permutations with
  opaque occlusion, foreground/occluded alpha, no-depth-write backgrounds and
  depth-ignoring overlays.
- Six teapot host checks pass. The shared cube/frame-schedule/sprite checks
  also pass. The glossy S3 firmware build and flash pass.

P4 wiring/defaults are provided; this showcase has not been hardware-tested on P4.
This showcase was approved on hardware before work began on the texture example.
