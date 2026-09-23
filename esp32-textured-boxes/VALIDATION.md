# Texture showcase validation - 2026-09-23

ESP32-S3 revision 0.2, 240 MHz, 8 MiB octal PSRAM at 80 MHz; 480x320 panel,
80 MHz SPI, eight-row DMA queue, half-width alternating field buffers.
Built with ESP-IDF 6.0.1 and flashed to the connected S3.

## Texture size and memory placement

The final scene uses a plain 200-unit cube at Z=520, yawing at 65 degrees/second
with rocking on the other axes. Mapping/filtering changes every three seconds.
All four modes share one texture. Separate 24-second serial captures measured:

| Texture | Storage sampled | Field FPS | Observed render time |
| --- | --- | ---: | ---: |
| 64x64, 8 KiB | Flash | 59.99-60.00 | 10.76-14.18 ms |
| 256x256, 128 KiB | Flash | 43.37-58.11 | 15.50-23.06 ms |
| 128x128, 32 KiB | Flash | 59.99-60.00 | 11.35-14.93 ms |
| 128x128, 32 KiB | Internal DRAM | 59.99-60.00 | 10.64-14.03 ms |

The 128x128 flash and DRAM captures each contain 22 reporting windows across
all four modes. Their mean render times are 13.03 and 12.16 ms respectively:
about 0.87 ms (6.7%) less render time with DRAM. Both reach the 60-field cap.
These are animated workloads with minor timing/pose differences, not fixed-pose
microbenchmarks or guaranteed frame rates. Mode transitions can cross a window.

The final startup log confirms `128x128, 32768 bytes, internal DRAM` and
60,435 bytes of internal heap free after runtime setup, versus 93,223 with
flash sampling. PSRAM free is unchanged at 8,232,208 bytes. The depth buffer
uses PSRAM. The texture is allocated once with internal/8-bit capabilities;
allocation failure retains flash sampling. No allocations occur on mode changes.

The final firmware occupies 0x5c3d0 bytes of the 1 MiB app partition;
0xa3c30 bytes (64%) remain free. The original texture also remains in flash.
Scanout measures 16.24-16.25 ms. No idle-recovery yields, allocation failures
or resets occurred during the final capture. The cube has 12 source triangles;
the overlay counts triangles actually accepted for rasterization per field.
FPS means render fields/second: each physical LCD row updates on alternate fields.

## Correctness

- Native cube preview and texture-option tests pass with the final 128x128 asset.
  All four modes are rendered at the same pose for visual comparison.
- Mode timing, clock wrap, framebuffer guards, cube triangle count, and FPS/TRIS/
  TRI/S sampling with constant and varying triangle counts are checked.
- Independent material mapping and texture filtering are checked against analytic
  reference texels. Indexed textures safely retain nearest sampling.
- Texture-option checks pass for all four build-capability combinations, including
  attempts to enable a feature compiled out of the engine.
- The existing affine interpolation oracle passes after correcting its field-row
  parity reference to use absolute row parity, matching renderer addressing.
- Six teapot host checks and three shared template/runtime checks passed with the
  engine texture changes. Final S3 build, flash and runtime verification pass.

P4 defaults are included; this showcase has not been tested on P4 hardware.
Approved on S3 before starting the tropical island showcase.

Counter update: the overlay now includes mean render MS, and TRI/S is based
on summed rendering time. Earlier throughput figures above used elapsed field
time and remain historical measurements. The shared tests verify independence
from frame pacing, weighted totals, sample resets and zero-duration safety.


## Painter default after depth audit

The measurements above describe the earlier depth-buffered builds. The final
crate now uses `Z_BUFFERING=0`, `FAST_Z=1`, with stable painter buckets.
It is a single convex backface-culled cube. Across 120 poses (30 seconds,
over five turns), all four modes and both parities, only 850 of 36,864,000
stored colour samples differ from depth. Differences are confined to shared
edges; the worst view has 42 changed samples, at most five with a channel
difference over 32. No face-order artefacts were visible in the compared views.

Fresh S3 captures with the same 128x128 internal texture:

| Visibility | Mean reported render time | Range | Field FPS |
| --- | ---: | ---: | ---: |
| Depth buffer | 12.47 ms | 10.95-14.43 ms | 59.99-60.00 |
| Painter | 9.09 ms | 7.78-10.59 ms | 59.99-60.00 |

About 27% less render time in these animated samples, not a fixed-pose benchmark.
Both runs cover the same mode cycle; report windows can cross mode transitions.
The 153,600-byte depth allocation is removed. Startup free internal memory is
57,267 bytes and free PSRAM is 8,385,812 bytes. Texture placement remains DRAM.
Scanout is 16.22-16.23 ms; no recovery yields or resets were observed.
The painter firmware is 0x5c6b0 bytes (64% of its app partition remains free).

The native test now renders all 120 poses in all modes with a null depth pointer,
checking both field guards. Both preview and analytic texture-option tests pass.
