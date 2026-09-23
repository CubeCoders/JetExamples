# Cel / Teapot validation

Built with ESP-IDF 6.0.1 and flashed to the connected ESP32-S3 on 2026-09-23.
Jet's runtime cel controls and flat-colour fix are committed locally as
**f4a5b0b**. Firmware was built from these source changes before committing them.
The user visually approved this example on the S3.

## Native checks

Both Release CTests passed with assertions enabled. The integrated test renders
28 matched poses in both modes and both field parities. Depth results, geometry
coverage and background remain identical; 346,043 colour samples differ. Across
these poses, rendered teapot colours reduce from 443 to 180 (ambient and additive
gloss remain continuous, so the final image has more than four colours).
48 serial/parallel comparisons, colour/depth guards and mode transitions pass.
The four-view montage was visually inspected.

The quantisation test independently checks exact flat/Gouraud output at band
boundaries, disabling, zero/clamped bit counts, constant-normal Phong and unlit
bypass. All five existing car checks also pass with cel compiled out, including
constant-normal lighting, environment mapping and unlit spans.

## Hardware

A 34-second capture includes two full 14-second loops and the start of a third.
Across 24 reporting windows, cadence averaged **45.49 fields/s**,
ranging **41.89-47.63** with pose. Excluding the first report after each
switch, mean render times over the first two cycles were **21.76 ms smooth**
and **21.86 ms cel**. Reporting windows are 60 fields, not exact matched-pose
samples, so the small difference is not evidence of a speedup. The same Phong
normal/specular work and depth testing run in both modes.

The capture had no unexpected resets, panics or idle recovery yields. Scanout is
about 16.35 ms. FPS, render milliseconds, accepted triangles and actual render
triangles/second remain visible. The submitted mesh has 822 vertices and 1,560
triangles; the accepted raster count varies with culling and pose.

Startup memory: **92,247 bytes internal**, **8,167,772 bytes PSRAM** free. The
150 KiB depth buffer is retained for the intersecting teapot surfaces in both
modes. Cel shading itself adds no image/row buffer or full-screen pass. Firmware
is **0x63340 bytes**, leaving 61% of the 1 MiB app partition free. The existing
16 MiB physical / 8 MiB configured flash warning is unchanged.

P4 defaults are provided but were not tested on hardware.
