# Tropical island validation - 2026-09-23

ESP32-S3 revision 0.2, 240 MHz, 8 MiB octal PSRAM at 80 MHz; 480x320 panel,
80 MHz SPI, eight-row DMA queue and half-width alternating field buffers.
Built with ESP-IDF 6.0.1 and flashed to the connected S3.

## Correctness

The initial ocean revealed an engine depth-interpolation overflow: multiplying
32-bit barycentric weights by vertex depth wrapped on large projected triangles.
The coverage regression reproduced up to 5,868 missing pixels in a foreground
field. Depth interpolation now keeps the original integer path when the weighted
sum fits; larger faces use full-width weights and a per-triangle floating-point
reciprocal, anchoring depth per row and incrementing it across the span. It retains the engine's affine depth interpolation convention.

- The final ocean coverage test checks 240 camera samples and both field parities across
  the full 30-second distance cycle (two orbits), with zero missing foreground pixels. A solid test material
  separates coverage from reflection colour and overlay composition.
- Large-triangle depth is compared with a 64-bit weighted reference at projected
  extents of 100, 10,000 and 100,000 units; error is at most one depth unit.
- Twelve concurrent band configurations match serial colour, depth and unique
  triangle counts exactly, including odd row splits and picking results.
- Four orbit views check guards, immutable previous-field data and the flare
  becoming visible and hidden. The clock-wrap path is also checked.
- A reflection-cutoff regression changes every excluded source row and verifies
  identical output; changing allowed rows changes output. Disabling the cutoff
  retains the previous sampling behaviour. Tests pass in the simple fast span,
  unlit depth and lit depth renderer configurations.
- Shared overlay tests separate wall-clock FPS from rendering time, verify
  weighted TRI/S totals, different pacing, window resets and zero-duration safety.
- Three island, eight teapot, two crate and five shared host checks pass.
  S3 firmware builds pass for the island, teapot and crate.

## Hardware measurements

The depth-buffered version initially measured 44.12-58.44 fields/s with
16.94-22.40 ms rendering. The user requested painter's sorting unless a scene
needs a depth buffer. This scene's flat water lies below all terrain, so drawing
it in the background band is sufficient. The final configuration uses the
game's `FAST_Z=1`, `Z_BUFFERING=0`, unlit/no-texture fast spans, with no depth
allocation or depth-buffer traffic. The depth correctness fix remains covered
by the teapot's engine regression tests.

A 24-second S3 capture of the final painter-sorted scene, game ripple settings,
higher sun and varying camera distance measured:

| Measurement | Observed |
| --- | ---: |
| Cadence | 59.97-59.98 fields/s |
| Render time | 5.24-6.47 ms |
| Setup | about 1.78-1.85 ms |
| Scanout | 16.27-16.30 ms |
| Rasterized triangles at report time | 549-576 |
| Rendering throughput | about 80,600-107,400 tris/s |

The first report includes worker/startup settling. Subsequent render time is
5.24-6.14 ms and reported render throughput is about 91,100-107,400 tris/s.
Pose, projected coverage and flare visibility vary through the orbit. Render
TRI/S uses summed counts divided by summed render microseconds, excluding pacing
and scanout waits. It is a workload measurement, not a maximum-triangle claim.
FPS counts fields; physical LCD rows update on alternate fields.

At startup, internal heap free is 46,975 bytes and PSRAM free is 8,378,592 bytes.
The firmware is 0x4fd70 bytes, leaving 0xb0290 bytes (69%) of its 1 MiB app
partition free. No resets, allocation failures or idle-recovery yields occurred
during the capture. P4 hardware is untested.

The user confirmed the corrected water coverage and reflection placement looked
good on S3, then requested game ripple settings, a higher sun and camera distance
variation. Those changes and the subsequent painter's switch are now flashed.
The user observed the final counter exceeding 100k render tris/s.
