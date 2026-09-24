# Validation — 24 September 2026

The film is reviewed through native-rendered videos while the reviewer is remote.
Its 50–60 fields/s hardware target is still unmet in several scenes. The fixed
60 fps video exports are visual references, not S3 performance measurements.

## Current motion and camera checks

- The 570-unit coupe represents a 4.5-metre car. The road and wheel animation
  use 70 MPH (6.95 car lengths/s) before the speedometer insert and 88 MPH
  (8.74 lengths/s) afterwards. The tight right turn slows to about 25 MPH.
- Sixteen eastbound boulevard blocks recycle around the exit tracking rig.
  The approach road moves out of view without recycling into the boulevard.
  A regression checks continuity at the end of the turn and backward seeking.
- Lane changes and overtakes have been retimed for the new speed. Hero/traffic,
  police/traffic and camera/vehicle envelope checks pass through the timeline.
- Camera clearance includes actual street triangles, covering projecting neon
  signs and awnings within the combined shop meshes. A 60-unit clearance sphere
  covers the near-plane corners. These checks run at 100 Hz; building volumes
  and hero sightlines are also checked during the 20 Hz rendered sweep.
- Flight pitch follows the velocity vector through the short climb ramp, then
  holds about seven degrees nose-up. The body, canopy, wheel pod transforms,
  environment mapping and glow positions share this orientation. The road
  shadow stays horizontal. The final cut keeps the same forward/climb velocity.
- One shallow dashboard contains two flush displays and a narrow scanner.
  There is no overlapping console tower or yoke. The speed display changes a
  scene-local 16 KiB texture only when the integer MPH value changes; scene
  allocation prefers PSRAM. The cabin remains visible from the driver camera.

These checks validate the authored sequence, not arbitrary new geometry or
camera paths. Billboards use their authored envelopes for the clearance check.
Painter sorting and glow sprite occlusion remain scene-authoring constraints.

## Regression coverage

- Eight native tests pass in both normal and reference renderer builds: complete
  film, offscreen clipping, unlit spans, texture options, water reflection,
  painter buckets, minimal FPS overlay and sealed vehicle geometry.
- All 2,021 rendered timeline hashes match with the material span optimizations
  and unlit-normal shortcut disabled. Representative serial/parallel frames
  match; packed buffers retain their guards through the full sweep.
- Three scene ownership cycles pass, including release of alternate LOD meshes,
  cockpit display storage and the final black fade.
- Flight direction is checked against the derivative of the trajectory during
  the climb ramp and after it settles, within the engine's integer-degree pose.
- The structural vehicle test welds equal-position edges and requires exactly
  two opposite uses of every edge. The body, canopy/glass and capped wheels pass;
  intentional decals, shadow and additive rings are excluded.
- The separate desktop-quality UV precision regression passes.
- Earlier shared renderer changes also passed the car (5), lighting teapot (8),
  island (3), runtime/cube/overlay (5) and depth-teapot/sorting (6) suites. No
  shared Jet code changed in this cockpit/motion revision.

## S3 run

The ESP-IDF build passes without a stack-frame warning over 2 KiB. The revision
was flashed on COM6 and completed all eleven cuts, the black hold and intentional
reboot, with no captured panic or heap error. The lowest reported internal free
heap at a scene boundary was about 18 KiB, with a 17 KiB largest block.

Before the lens-animation pass below, mean sampled cadence compared the sealed-car
revision with the revised cockpit,
calibrated MPH movement, sign clearance and pitched flight. These are serial
window means, not per-frame minima; windows can straddle cuts.

| Cut | Previous | Current fields/s |
| --- | ---: | ---: |
| 01 THE RIVER | 51.8 | 51.8 |
| 02 RAIN DISTRICT | 30.1 | 30.1 |
| 03 THE COURIER | 45.8 | 51.3 |
| 04 REAR VIEW | 36.3 | 33.9 |
| 05 NO DRIVER | 34.3 | 38.4 |
| 06 EIGHTY EIGHT | 59.8 | 59.5 |
| 07 BOULEVARD | 44.6 | 45.9 |
| 08 PURSUIT | 45.1 | 44.8 |
| 09 FLIGHT MODE | 43.9 | 42.1 |
| 10 IGNITION | 51.1 | 39.0 |
| 11 ABOVE IT ALL | 48.0 | 56.0 |

The simpler cockpit improves its mean, but rain, front tracking and the climb
remain well below the target. Faster travel and changed framing alter visible
geometry and pixel coverage, so this is not an isolated renderer benchmark.
Hardware uses packed half-width alternating fields: a physical row updates at
half the field rate.

## Video review

The packed-field export reconstructs 480×320 fields into 960×640 video. The
desktop-quality export renders full 2880×1920 colour and depth buffers, then
downsamples to 1920×1280. It uses bilinear perspective textures, full-detail
meshes and checked guards around all three buffers on every frame. The quality
target retains RGB565 and Jet's lighting; it adds no field reconstruction or
hardware FPS overlay.

Both exports decode without errors. The quality export contains 6,120 frames,
102 seconds at 1920×1280 and 60 fps. The packed-field export is also 102 seconds.
The review checks cover the simplified
cockpit, complete right-turn framing, street passage and pitched final fly-by.
The full-resolution render uses Jet's optional `JET_HIGH_PRECISION_UVS` to avoid
texture interpolation overflow on very large projected triangles. That option
remains disabled on S3 and adds no firmware pixel cost.

## Earlier crash corrections

The startup reset came from assigning a temporary 200-slot particle system on
the runtime stack. Resetting its existing pool in place removes the 7 KiB
temporary from the 8 KiB task stack.

The reset after 88 MPH was framebuffer corruption from a horizontally empty
clipped triangle: integer truncation could turn a negative half-width span into
one pixel. The rasterizer now rejects empty X bounds before packed addressing.
Its regression guards both buffer ends and covers all screen edges and parities.

## Rendering limits

The S3 uses painter sorting with 128 depth buckets and no depth allocation.
Intersecting geometry can still require subdivision, explicit ordering or depth
testing. The glass environment map approximates a distant city panorama, not
live local reflections. Glow sprites have no general scene occlusion. Street
draw distance is 9,000 units, with 3,200-unit facade LOD and 8 KiB of persistent
filtered facade textures in internal DRAM. Large scene allocations prefer PSRAM.
P4 configuration exists but this film has only been run on S3 hardware.


## Animated lens pass

Three linear field-of-view pulls add optical zoom to the existing camera paths:
the gauge narrows from 62 to 46 degrees over five seconds, the launch widens
from 54 to 70 over five seconds, and the final approach opens from 34 to 68
over two seconds. The final lens then holds through the fly-by and fade. The
car's MPH and trajectory remain unchanged. These use the floating-point camera
API; they add no depth-of-field blur pass or image allocation.

All eight native tests pass, and all 2,021 normal/reference frame hashes match
with the animated projections. The existing near-plane clearance envelope
still covers these lenses, which are narrower than the film's 74-degree maximum.

The lens revision was flashed on COM6 and completed all eleven cuts and the
intentional restart without a captured panic or heap error. Sample means for
the gauge, launch and final cut were 56.5, 40.9 and 55.8 fields/s respectively;
the overall 50 fields/s minimum remains unmet. No image buffer was added.

The updated full-quality video decodes to 6,120 frames at 1920×1280 and 60 fps;
unchanged sequences reuse the preceding export. A separate 19-second review
joins the gauge, takeoff and fly-by and decodes to 1,140 frames. Neither video's
playback rate represents the S3's measured speed.
