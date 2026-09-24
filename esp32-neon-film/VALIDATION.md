# Validation â€” 24 September 2026

The film is a work in progress, with visual review now conducted through
native-rendered MP4s because the reviewer is remote. The 50 fields/s minimum
target is not yet met. Do not present the 60 fps export as a hardware benchmark.

## Confirmed checks

- ESP-IDF S3 build and COM6 flash; multiple full eleven-cut loops followed by
  the deliberate black-hold restart. No earlier reset in those captured runs.
- Native eight-test suite: complete film, offscreen clipping, unlit spans,
  texture options, water reflection, extended painter buckets, minimal FPS overlay and closed vehicle geometry.
- 2,021 timeline poses at 20 Hz, guarded packed field buffers, camera/building
  clearance and camera-to-car sightline checks, hero/traffic and police/traffic oriented-box
  overlap checks, clearance from camera rigs to vehicle bodies, three complete scene ownership cycles and black end fade.
- Representative serial/parallel pixel comparisons; complete timeline hashes
  match when the new material span paths and unlit-normal shortcut are disabled.
- Shared renderer regressions: car (5), lighting teapot (8), island (3) and
  repository runtime/cube/overlay tests (5) passed. The default-bucket runtime
  tests (5) and depth-teapot/depth-sorting tests (6) were rerun after the bucket change.
- 102-second RGB24-to-H.264 export: 6,120 decoded frames at 960Ã—640, generated
  from the 480Ã—320 field reconstruction without interpolation or motion blur.

## Crash corrections

The startup reset came from assigning a temporary 200-slot ParticleSystem on
the runtime stack. Resetting the existing pool in place removes that large
temporary. The ESP32 build warns about stack frames larger than 2 KiB.

The reset after 88 MPH was framebuffer heap corruption. A horizontally empty
clipped triangle could turn a negative half-width span into one pixel because
integer division truncates toward zero. The rasterizer now rejects empty X
bounds before the packed-slot calculation; the dedicated regression guards
both ends of the buffer and covers all four offscreen boundaries and parities.

## Performance and limits

The original-coupe S3 run completed without crashes but ranged roughly from
26 to 60 fields/s. Grouped storefronts and better memory placement lifted the
measured boulevard mean from about 41 to 51 and cockpit mean from 36 to 40;
rain remained around 34. These are serial sample means, not per-frame minima.
Hidden-face culling then lifted the river mean to 52, the overhead car to 50,
front tracking to 55, boulevard to 52 and pursuit to 48. The rain street (35),
cockpit (40) and hover conversion (42) still need work. All eleven cuts and the
intended restart completed on S3 with that revision. The subsequent review
video also moves three camera rigs clear of traffic and corrects police spacing.
The enclosed-cabin revision adds the roof, windscreen frame, door cards, console
and unattended yoke. Its native tests and all 2,021 optimized/reference timeline
hashes pass; both the cockpit excerpt and full MP4 decode without errors.
The hardware figures above predate this cabin geometry.

The next camera pass uses oblique overhead/front-quarter views, an angled
cockpit and gauge insert, offset chase compositions and a diagonal city overview.
Street visibility then increases from 5,200 to 9,000 units. Recycled scenery
extends in the shot's viewing direction; sixteen blocks cover forward and rear
tracking. The rain street has seven detailed blocks plus seven distant facades.
The right-turn boulevard now has 28 blocks to cover the entire car path.

Extending the far plane initially exposed coarser painter ordering on the hood
and instruments. Optional `JET_SORT_DEPTH_BUCKETS` retains the default 64 in Jet;
this film uses 128, preserving finer near-field ordering without a depth buffer.
The regression checks overlapping layers at a 9,000-unit far plane, draw-band
priority and stable equal-depth ordering. The complete 2,021-pose optimized and
reference hashes match with this revision, and the 102-second MP4 decodes cleanly.

Painter sorting remains an authored-scene compromise: arbitrary new intersecting
geometry may need subdivision, ordering changes or depth testing. Environment
mapped glass approximates a distant city panorama, not live local reflections.
Projected glow sprites do not perform general scene occlusion. The camera
checks cover building volumes, vehicle envelopes and subject sightlines; they are not a complete
collision system for every lamp, cable or awning.


## Contrast and distant facade revision

The final contrast pass grades procedural texture outputs and authored colours
before rasterization, preserving the additive glow falloff. Distant storefronts
use Jet's existing mesh LOD at 3,200 units, with the same shop artwork and reduced
projecting detail. The four 32Ã—32 filtered facade textures use 8 KiB of internal
BSS (verified in the S3 link map). LOD meshes are owned separately from renderable
scene objects and released at scene cuts; the ownership-cycle check verifies
that no alternate meshes survive into the opening scene.

All seven native tests and the 2,021 optimized/reference timeline hashes pass
for this revision. Its complete 102-second H.264 export decodes without errors.


### Latest S3 comparison

The final contrast/LOD revision completed all eleven cuts and the intentional
restart on COM6 without a captured panic or heap error. Mean sampled cadence
compares the first extended-distance build against the LOD/contrast revision.
These are serial window means, not per-frame minima; windows may straddle cuts.

| Cut | Extended view | With LOD and contrast |
| --- | ---: | ---: |
| 01 THE RIVER | 51.3 | 51.4 |
| 02 RAIN DISTRICT | 28.7 | 29.7 |
| 03 THE COURIER | 49.1 | 52.4 |
| 04 REAR VIEW | 31.4 | 38.2 |
| 05 NO DRIVER | 30.5 | 34.6 |
| 06 EIGHTY EIGHT | 57.9 | 59.6 |
| 07 BOULEVARD | 43.8 | 48.8 |
| 08 PURSUIT | 46.3 | 48.6 |
| 09 FLIGHT MODE | 43.2 | 46.6 |
| 10 IGNITION | 46.3 | 51.6 |
| 11 ABOVE IT ALL | 43.0 | 42.0 |

The extended view remains more expensive than the earlier short streets.
The 50 fields/s minimum is still unmet, particularly in rain, cockpit and
front tracking; the fixed-60 native video is solely a visual review artifact.


## Sealed coupe, fly-by and desktop-quality export

The structural car mesh is closed: the regression welds equal-position edges
across the body, canopy/glass and wheels and requires exactly two oppositely
oriented uses of every edge. It rejects degenerate faces and excludes only
intentional surface decals, shadow and additive glow rings. Both road and
transformable variants pass. A separate 16-second turntable checks the body,
underside and wheel transition visually.

The white launch overlay is removed. Forward speed remains 1,800 units/s through
the climb and the final cut, with vertical speed 650 units/s and matching height
at the cut. The final scene rebases the forward coordinate, then the stationary
camera lets the coupe pass before a linear target tilt reveals the city and
the existing three-second fade. Camera clearance includes the wider hover pods.

The desktop quality build uses two full 2880×1920 colour buffers plus a depth
buffer, bilinear perspective textures, and full-detail meshes. All three buffers
have checked guard regions on every exported frame. It preserves RGB565 and the
engine's geometry/lighting; downsampling to 1920×1280 supplies spatial AA. Its
quality-only UV regression checks analytic samples for projected triangles whose
areas and reciprocal-depth products exceed the compact integer path's limits.
The new Jet option defaults off, preserving the firmware's arithmetic.


The final build passed all eight native tests and all 2,021 optimized/reference
timeline hashes match. The separate quality UV regression passes. The S3 build
has no frame-size warning over 2 KiB and completed all eleven cuts and its
intentional reboot on COM6 with no captured panic or heap corruption. The lowest
reported internal free heap was about 18 KiB, with a 17 KiB largest block.

The complete quality export decodes as 6,120 frames, 102 seconds, 1920×1280 at
60 fps. The updated packed-field review also decodes without errors. Neither
video's playback rate measures hardware speed.

Mean sampled S3 cadence after sealing the car and revising the ending:

| Cut | Previous | Sealed car / fly-by |
| --- | ---: | ---: |
| 01 THE RIVER | 51.4 | 51.8 |
| 02 RAIN DISTRICT | 29.7 | 30.1 |
| 03 THE COURIER | 52.4 | 45.8 |
| 04 REAR VIEW | 38.2 | 36.3 |
| 05 NO DRIVER | 34.6 | 34.3 |
| 06 EIGHTY EIGHT | 59.6 | 59.8 |
| 07 BOULEVARD | 48.8 | 44.6 |
| 08 PURSUIT | 48.6 | 45.1 |
| 09 FLIGHT MODE | 46.6 | 43.9 |
| 10 IGNITION | 51.6 | 51.1 |
| 11 ABOVE IT ALL | 42.0 | 48.0 |

The extra closed car surfaces add geometry cost to its exterior shots.
Rain and cockpit remain around 30–34 fields/s; the 50 fields/s minimum is still
unmet. Means come from serial windows that can straddle cuts, not frame minima.
The native quality path is a separate desktop build, and the new precision UV
option remains disabled in S3 firmware.
