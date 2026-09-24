# Validation — 24 September 2026

ESP 88 now contains twelve scenes over 116 seconds, followed by one second of
black and an intentional restart. Native review videos have a fixed playback
rate; they are not hardware frame-rate measurements. The 50–60 fields/s target
remains unmet in several scenes.

## Ordering and vehicle geometry

The S3 retains painter rendering without a depth allocation. Vehicle objects
request exact triangle-mean ordering inside the depth buckets they occupy.
This uses one boolean per bucket (130 bytes in this film) and in-place sorting.
Background and overlay bands, depth bias and stable exact-depth ties are retained.
It does not solve arbitrary intersecting polygons or cyclic overlap.

Traffic bodies now have recessed wheel wells and a closed chassis floor instead
of uninterrupted side panels passing through their wheels. A 72-angle unlit
mask comparison against per-pixel depth found large reductions in leaked wheel
pixels: the hero road/overhead/underside aggregate fell from 38,399 to 561.
Traffic road, overhead and underside cases fell from 11,853/17,264/42,328 with
the old body and refined sorting to 18/15/86 with the recessed geometry. These
are diagnostic image counts for the sampled poses, not a universal accuracy
claim; small edge disagreements remain.

Desktop quality uses the opt-in `JET_PERSPECTIVE_DEPTH` path: reciprocal Z is
interpolated before inversion, fixing depth errors on large sloping panels.
It requires depth enabled and `FAST_Z=0`, adds a pixel division, and is disabled
on S3. The separate high-precision UV option remains desktop-only too.

## Motion, weather and closing sequence

- A 570-unit car represents 4.5 metres: 70 MPH is 6.95 car lengths/s and
  88 MPH is 8.74 lengths/s. Hard braking leads into a 1.2-second sideways skid
  at about 24 MPH, with nose dip and suspension roll. Road wheels stay planted.
- The exit camera keeps facing back along the street while the car accelerates
  past it. The scene rebases into the moving tracking rig only after that pass.
  Camera translations are linear within each shot.
- Police begin far behind in the boulevard shot and close to gaps of 1,600 and
  2,400 units. Those gaps continue across the chase cut, which first establishes
  the group before closer viewpoints. Vehicle and camera clearance checks cover
  the added cars and the altered suspension pose.
- Flight retains forward velocity and pitches into the climb. The final flyby
  is staged 1,500 units lower to show more city; it fades to black. The waterfront
  returns at 101 seconds and pans throughout the credits: scene fade-in 101–103,
  text fade-in 103–105, fully readable 105–113, shared fade-out 113–116.
- Neon ribbons target three projected pixels to allow integer rounding, keeping
  at least two pixels across the slow opening pan. The same treatment covers
  shop trims, reflected trims and city ground lines. Far shop LOD has broader
  trim bands. Texture detail and other small geometry can still alias.
- Searchlights have wider, faster sweeps plus brighter cores. Fast world-aligned
  rain appears in every exterior shot, including reverse and aerial views.
  Cockpit/instrument shots remain dry. Road splashes use independently hashed
  X/Z positions and change location at each birth, avoiding the former lattice.

## Automated checks

Eight native tests pass: full film, offscreen clipping, unlit spans, texture
options, water reflection, painter buckets, minimal overlay and sealed hero mesh.
All 2,321 normal/reference timeline hashes match at 20 Hz, with guarded packed
buffers, representative serial/parallel comparisons and three ownership cycles.
Street triangles, camera/vehicle envelopes and traffic overlap are checked at
100 Hz. Building clearance and hero sightlines run in the rendered sweep.

Additional assertions cover the two-pixel skyline minimum, rain in exterior
shots and its absence in interiors, the car passing the exit camera, police gap
continuity, braking velocity, flight pitch, and the eight-second credit hold.
These checks validate the authored sequence, not arbitrary camera paths.

Both desktop precision tests pass: large projected UVs and analytic reciprocal-Z
depth. Shared renderer regression suites also pass: car (5), lighting teapot (8),
island (3), runtime/cube/overlay (5) and depth teapot (6).

## S3 run

The ESP-IDF build succeeds. Flashing on COM6 was verified, then a captured run
completed all twelve scenes, the credit hold, fade and intentional reboot,
without a captured panic or heap error. The lowest reported internal free heap
at a scene boundary was 19,615 bytes, with a 17 KiB largest block.

The following are means/ranges of serial sampling windows in that full loop,
not per-frame extrema. Windows can straddle scene boundaries. Increased rain,
stronger lighting and more detailed traffic cost performance; current results
are below the target. Physical LCD rows update at half the field rate.

| Scene | Mean fields/s | Sample range |
| --- | ---: | ---: |
| 01 THE RIVER | 43.7 | 40.6–45.9 |
| 02 RAIN DISTRICT | 27.9 | 23.6–38.8 |
| 03 THE COURIER | 38.9 | 25.8–42.4 |
| 04 REAR VIEW | 30.4 | 28.6–35.1 |
| 05 NO DRIVER | 34.4 | 27.9–36.0 |
| 06 EIGHTY EIGHT | 58.3 | 49.8–60.0 |
| 07 BOULEVARD | 33.0 | 28.4–41.5 |
| 08 PURSUIT | 33.0 | 23.6–41.6 |
| 09 FLIGHT MODE | 34.6 | 33.5–35.8 |
| 10 IGNITION | 30.0 | 28.9–31.1 |
| 11 ABOVE IT ALL | 47.8 | 40.4–57.5 |
| 12 ESP 88 | 35.8 | 33.3–38.8 |

## Video review

The S3-matched export reconstructs 480×320 packed fields into 960×640 video.
Desktop quality renders full 2880×1920 RGB565 colour and depth buffers, with
bilinear perspective textures and full-detail meshes, then downsamples to
1920×1280. It checks guards around both scene buffers, the presentation buffer and depth
every frame. Overlays compose onto the presentation copy, so water never samples
credit text or a previous fade. This matches hardware scanout separation.
Neither export fabricates hardware FPS, and both are silent visual reviews.

Each final export contains 7,020 frames: 117 seconds at 60 fps. Both videos have passed a complete decode, with contact sheets inspected for
rain, the pass-by, police continuity and the waterfront credits.

## Earlier crash fixes and remaining limits

Particle-pool reset avoids the former 7 KiB stack temporary. Empty horizontally
clipped spans are rejected before packed addressing, preventing the old reset
after the speedometer cut. The corresponding buffer guards remain enabled in
native tests.

Glow sprites have no general scene occlusion; environment mapping is a distant
panorama rather than live local reflections. Painter ordering still constrains
intersecting geometry. P4 configuration exists but hardware validation is S3-only.
