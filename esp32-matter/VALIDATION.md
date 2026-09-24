# Validation — 24 September 2026

MATTER was built with ESP-IDF 6.0.1 for the connected ESP32-S3, flashed to COM6,
and observed through all ten movements, the final fade and deliberate reboot.
No panic, watchdog reset or heap corruption appeared in the captured full loop.

## Hardware cadence

These are means and ranges of serial reporting windows, not per-frame minima.
Windows straddling a cut can include both scenes and loading time. The numeric
display counts completed render fields per second; each physical LCD row is
updated at half that rate. Firmware uses two packed half-width field buffers,
nearest textures, a parallel raster worker and painter ordering without depth.

| Movement | Mean fields/s | Reporting-window range |
|---|---:|---:|
| Pressure | 35.9 | 31.7–39.2 |
| Counterweight | 48.8 | 35.7–60.0 |
| Paper Weather | 41.0 | 39.3–42.3 |
| Quicksilver | 37.8 | 34.9–39.5 |
| Colour In Space | 36.1 | 35.0–36.8 |
| Botanica | 56.3 | 41.2–59.8 |
| Interference | 43.2 | 41.3–45.2 |
| Reliquary | 50.3 | 40.5–52.7 |
| The Gyre | 53.1 | 42.2–54.9 |
| Matter | 37.2 | 35.8–39.3 |

Lowest reported internal free memory at a scene boundary: 17,855 bytes.
Application binary: 403,872 bytes. This is inspired by 64K intros,
not a 64K size-constrained executable. Binary SHA-256:
`198c227ae2ca2b5552f6d70fbb04bba0cb17f62518325ebea2b9432b4adeb7fb`.

The broad aim is 35–60 fields/s. A few reporting windows still fall into the low
30s; this is documented rather than presenting a guaranteed minimum cadence.

## Automated and visual checks

- Passed the complete 180-second native timeline at 10 Hz: 1,801 poses, each
  rendered as both S3 field parities with guard regions around both buffers.
- Passed three complete scene ownership cycles, serial/repeated/parallel
  render comparisons, backwards seeking, dynamic bounds, triangle references,
  final black output and the 181-second restart condition.
- Built the optional desktop renderer and rendered all ten movements with
  full 2880×1920 colour/depth buffers and intact guard regions.
- Inspected S3-layout frames throughout every movement, including the pale
  scenes' dark captions, the faster object rotations and readable final card.
- Rechecked the revised Pressure, Counterweight, Quicksilver, Interference and
  Gyre compositions at six points throughout each movement.
- Swept the revised collision-sensitive motion at 60 Hz. Conservative minimum
  clearances in scene units: gears/rings 11.83, opening rings/plinth 47.91,
  cubes/architecture 36.81, chrome rings/plinth 34.74, knot/tunnel bore 7.66.
- Fully decoded the final S3-quality MP4: 10,860 frames, 181 seconds, 60 Hz,
  960×640 nearest upscale from 480×320 reconstructed alternating fields.

The video uses fixed 60 Hz sampling to review the artwork. It does not simulate
hardware cadence or include a fabricated FPS counter. Full-resolution desktop
video is available through the renderer; only its ten-scene smoke render was
generated in this validation pass.

## Performance choices

Closed industrial gears cull hidden faces. Their two rounded bevel bands use
continuous normals and Phong specular shading, while broad faces remain flat.
The surrounding hoops retain Gouraud lighting. Cylinders use a single side band
and genuine cap fans rather than vertical subdivisions. Scene-wide precise
sorting avoids an additional in-place object sort, keeping animated topology
and equal-depth submission order stable. Folded-paper triangles have their own
normals, and shared wave coordinates need only 41 samples per frame. The liquid
surface separates spatial and temporal sine bases, using six trigonometric
evaluations per frame while retaining analytical normals. Reflection transforms
are calculated once per object; rings and tunnel profiles use compact geometry.

Painter sorting still approximates intersecting surfaces. The room texture is
a procedural environment-map illusion, not a live reflection. Whole-degree mesh
rotation remains an engine constraint, so exposed rotations are deliberately
brisk. There is no soundtrack in this exhibition.
