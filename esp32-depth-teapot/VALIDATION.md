# Depth comparison validation - 2026-09-23

ESP32-S3 revision 0.2, 240 MHz, octal 8 MiB PSRAM at 80 MHz, 480x320 output,
half-width alternating fields, shared dual-core renderer and queued SPI scanout.

The current version switches every seven seconds and rotates once per phase
(about 51.4 degrees/second), with fixed glossy Phong shading. This replaced an
initial three-second comparison after hardware feedback.

## Correctness

- Six native tests pass: animated depth comparison, runtime toggle and clipping,
  depth addressing, opaque/blended depth ordering, wide-triangle depth, and water
  reflection regression tests, all built with runtime depth support enabled.
- The comparison checks 120 poses, both modes and field parities, null and stale
  depth storage, guard memory, switch boundaries and clock wrap. Its 128 parallel
  comparisons include odd band boundaries and match serial colour/depth output.
- Independently compiled static depth and static painter builds were compared
  against the corresponding runtime kernels over 120 poses, three shading modes
  and both field parities: zero changed samples out of 27,648,000 per kernel.
- Existing lighting (eight), texture (two), and island (three) tests also pass
  with the runtime feature disabled, including island water coverage.

## S3 measurements

The seven-second version was built, flashed and captured over serial. Settled
reporting windows (omitting the first window after each switch) show:

| Mode | Render time | Field FPS |
| --- | ---: | ---: |
| Painter | 17.13-18.87 ms | 52.81-58.18 |
| Depth testing | 19.91-22.80 ms | 43.69-50.01 |

Animated poses and variable reporting windows affect these ranges. This is not
an isolated memory benchmark: the depth path also sorts opaque triangles in
the opposite order to reject hidden pixels before expensive Phong shading.
The painter path has to shade those pixels before covering them.

Scanout is 16.28-16.31 ms. No recovery yields or resets were observed. Startup
free internal memory is 84,671 bytes and free PSRAM is 8,167,772 bytes. The depth
buffer is 153,600 bytes in PSRAM and is retained in the painter phase. There is
not enough free internal RAM for the current full-height depth buffer.
Firmware size is 0x5ca00 bytes; 64% of the 1 MiB app partition remains free.

P4 defaults are included but have not been built or hardware-tested for this
example. Visual review of the seven-second version is pending on S3.
