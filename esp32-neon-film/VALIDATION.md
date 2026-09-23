# Validation — 23 September 2026

The film is a work in progress, with visual review now conducted through
native-rendered MP4s because the reviewer is remote. The 50 fields/s minimum
target is not yet met. Do not present the 60 fps export as a hardware benchmark.

## Confirmed checks

- ESP-IDF S3 build and COM6 flash; multiple full eleven-cut loops followed by
  the deliberate black-hold restart. No earlier reset in those captured runs.
- Native six-test suite: complete film, offscreen clipping, unlit spans,
  texture options, water reflection and minimal FPS overlay.
- 2,021 timeline poses at 20 Hz, guarded packed field buffers, camera/building
  clearance and camera-to-car sightline checks, hero/traffic and police/traffic oriented-box
  overlap checks, clearance from camera rigs to vehicle bodies, three complete scene ownership cycles and black end fade.
- Representative serial/parallel pixel comparisons; complete timeline hashes
  match when the new material span paths and unlit-normal shortcut are disabled.
- Shared renderer regressions: car (5), lighting teapot (8), island (3) and
  repository runtime/cube/overlay tests (5) passed.
- 102-second RGB24-to-H.264 export: 6,120 decoded frames at 960×640, generated
  from the 480×320 field reconstruction without interpolation or motion blur.

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

Painter sorting remains an authored-scene compromise: arbitrary new intersecting
geometry may need subdivision, ordering changes or depth testing. Environment
mapped glass approximates a distant city panorama, not live local reflections.
Projected glow sprites do not perform general scene occlusion. The camera
checks cover building volumes, vehicle envelopes and subject sightlines; they are not a complete
collision system for every lamp, cable or awning.
