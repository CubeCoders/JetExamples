# REPEAT validation - 2026-09-24

ESP32-S3 revision 0.2, 240 MHz, 8 MiB octal PSRAM at 80 MHz, 64 KiB data
cache, 480x320 RGB565 output, half-width alternating fields, dual raster
workers and queued SPI scanout. ESP-IDF 6.0.1; release/performance build.
Jet dependency: `b412c87` (instancing is opt-in).

## Memory and animated performance

The 15-rotor comparison reports **348,000 -> 23,200 bytes** of stored mesh
payload, saving **324,800 bytes / 317.2 KiB / 93.3%**. This includes vector
capacity and packed-position/reuse caches, and excludes Object/instance
metadata, allocator overhead, labels, renderer working storage and framebuffers.
The copies have independent packed caches as well as independent geometry.

After the renderer has warmed up, serial reports settle at 629,816 bytes of
PSRAM used in the copy phase and 301,412 bytes in the shared phase. Free internal
RAM settles at 59,175 and 58,643 bytes respectively. These are whole-heap readings
at mode-switch boundaries, not mesh-only figures. The first startup readings
precede renderer queue/scratch/worker allocations and must not be compared with
later phases. Neither representation accumulates memory over repeated switches.

The normal animated demo was observed for 90 seconds before the scaling work
and again after restoring the final firmware. Settled reporting windows are
about **37.8-38.3 fields/s with copies and 40.5-41.2 with shared meshes**,
roughly 26.2 versus 24.4 ms of rendering. Scanout stays about 16.5 ms. No panic,
watchdog reset or idle-recovery event was observed. The eight-second phases use
the same motion, lighting and camera; transition windows can mix both modes.

## Repetition scaling

An isolated instrumented copy of the example held 1, 2, 4, 8, 12 and 15 rotors
at three matched poses (local times 0.4, 1.3 and 2.2 seconds). Counts use the first
N positions in the existing grid, so the image footprint changes with count but
is identical between memory strategies at each count. Each pose discards 12
warmup fields and averages 36 timed fields; the sweep repeats three times.
The normal scanout, field cadence and adaptive second raster worker remain active.
All three variants produce matching framebuffer hashes at all 18 count/pose pairs.

Mean complete render time (lower is better):

| Rotors | Copies, capability off | Copies, capability on | Shared instances | Reduction vs capability off |
| ---: | ---: | ---: | ---: | ---: |
| 1 | 2.034 ms | 2.028 ms | 1.940 ms | 4.6% |
| 2 | 3.574 ms | 3.591 ms | 3.343 ms | 6.5% |
| 4 | 6.839 ms | 6.864 ms | 6.245 ms | 8.7% |
| 8 | 13.940 ms | 14.011 ms | 12.509 ms | 10.3% |
| 12 | 21.495 ms | 21.565 ms | 19.714 ms | 8.3% |
| 15 | 26.134 ms | 26.277 ms | 24.276 ms | 7.1% |

The capability itself adds roughly **0.3-0.6%** render time in the 2-15-rotor
copy controls. Sharing more than offsets it here. At 15 rotors, mesh preparation
falls from about 13.1 to 11.1 ms, while raster work remains roughly 13.1 ms.
At small counts the 60-field cap hides the difference in FPS; compare render MS.

The one-rotor timing is especially sensitive to worker scheduling: most of its
apparent benefit is in raster time, despite no geometry reuse between placements.
Do not interpret that as a repeatable cache win. There is no universal crossover
at 15 instances. The more convincing gains begin at two rotors in this sweep.

The measured percentage also does not grow monotonically with repetition. The
scene's preparation/raster balance and the adaptive worker's band split change
as it gets larger. Bytes saved are not equivalent to bytes of avoided PSRAM
traffic. No hardware cache-miss counters were collected.

## Mesh complexity and lighting

A second sweep compares a 24-vertex/12-triangle cube with the existing
400-vertex/200-triangle rotor, at 1, 4 and 15 placements, using unlit, flat and
Phong materials. Flat uses diffuse lighting without specular; Phong retains the
demo's gloss. Two matched poses (0.4 and 1.3 seconds), 12 warmup fields and 24
timed fields per pose are repeated twice, reversing copy/shared order in the
second pass. Both modes have the instancing capability compiled in.

All 36 mesh/shading/count/pose combinations have identical framebuffer hashes
across both modes and both orders. Camera, placement, materials and raster
coverage match within each pair. Cubes and rotors have different silhouettes,
so compare the paired saving within a mesh, not their absolute render times.
The cube's allocated payload is 1,392 bytes per copy and 1,744 bytes for the
shared prototype: retained vector capacity also matters. The rotor is 23,200
bytes in either representation. Neither comparison includes instance metadata.

Results for **15 placements**, averaging both poses and both orders:

| Mesh | Lighting | Copies render | Instances render | Render reduction | Copies setup | Instances setup |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| Cube | Unlit | 2.647 ms | 2.633 ms | 0.5% | 0.685 ms | 0.673 ms |
| Cube | Flat | 5.579 ms | 5.528 ms | 0.9% | 0.796 ms | 0.727 ms |
| Cube | Phong | 8.053 ms | 7.900 ms | 1.9% | 0.780 ms | 0.711 ms |
| Rotor | Unlit | 20.678 ms | 19.636 ms | 5.0% | 11.143 ms | 10.047 ms |
| Rotor | Flat | 25.097 ms | 23.064 ms | 8.1% | 13.555 ms | 11.117 ms |
| Rotor | Phong | 27.908 ms | 25.827 ms | 7.5% | 13.214 ms | 11.072 ms |

Four rotors already save 7.0% unlit, 12.3% flat and 9.8% Phong. Cubes change
by about 1% or less at four placements. Individual light-workload samples show
substantial raster timing changes when the adaptive worker chooses a different
band split; averaging both orders is important. The sub-2% cube results should
be treated as little practical benefit, not a reliable optimization target.

The evidence supports **mesh working-set size plus reuse**, rather than count
alone. The normal-free unlit path benefits less than the lit rotor paths. In
Jet, flat lighting still consumes normals; unlit/emissive materials skip their
reads and lighting work. Phong still transforms normals even when the equal-
normal shortcut avoids interpolating them across these flat faces. Material
eligibility scans and transform composition also differ between the modes.
Cache locality is a plausible explanation, not a directly measured miss rate.

Keep the practical advice memory-first: expect a modest CPU cost, measure the
target workload, and treat any speed gain as scene-dependent. Many tiny props,
unique/deforming meshes, or interleaved unrelated geometry need not behave like
this adjacent repetition of one larger mesh. These results do not reverse the
earlier decision to leave ESP 88 and MATTER's geometry unconverted.

Raw per-pose readings are in [scaling.csv](assets/scaling.csv) and
[complexity.csv](assets/complexity.csv). Each row is an average over the timed
fields, not one frame; setup is complete render time minus raster time. The
CSV retains both ordering passes and framebuffer hashes. The unmodified animated
firmware is what remains on the S3, not the fixed-pose benchmark.

## Correctness and build checks

- The actual example matches copy/shared scene pixels and triangle counts at
  eight poses and both field parities. Concurrent raster bands match serial
  output, guard pixels remain intact, and 64 representation switches release
  the previous prototype without retaining it.
- Two engine fixture configurations each pass 144 expanded-versus-instanced
  frame comparisons, lifetime/copy/baking checks and immutable topology checks.
  The additional configuration enables triangle sorting, perspective textures,
  bilinear filtering and picking, including owner/prototype/instance attribution.
- All five existing shared-runtime/template host tests pass with instancing
  disabled. A source audit confirms the disabled branches of the modified engine
  files retain the original code (apart from whitespace/default flag declaration).
  No existing showcase configuration enables the new capability.
- S3 firmware builds successfully: 0x4f1f0 bytes, 69% of the 1 MiB app slot free.
  P4 defaults are included but this particular example has not been built or
  hardware-tested on P4.

Preview counters are unmeasured and desktop mesh-byte figures use the host ABI.
The S3's actual counters and memory logs are the basis for the figures above.
