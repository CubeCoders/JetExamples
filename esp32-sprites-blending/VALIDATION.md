# After Hours validation

Built with ESP-IDF 6.0.1 and flashed to the connected ESP32-S3 on 2026-09-23.
Jet remains at **fffbf63**; no dependency or shared-runtime changes were needed.
The firmware version label records the preceding examples commit because the
new project had not yet been committed when it was built.

## Native checks

The Release CTest passed with assertions enabled. It checks that every mirror
copy negates vertex/position Y and reverses winding, then renders matching poses
through all construction stages. The floor layer visibly differs, additive
meshes never reduce any RGB565 channel, and enabling halos leaves the world
framebuffer unchanged while changing the subsequent sprite composite.

Mirrored-quarter dimensions, colour-buffer guards, both field parities, six
serial/parallel scene comparisons and every stage boundary are also checked.
The six-view montage was visually inspected after the final composition changes.
The controlled camera arc is part of the authored effect; arbitrary full orbits
and intersecting transparent geometry are not claimed by this example.

## S3 capture

A 49-second serial capture includes the complete 42-second reveal loop and the
return to the finished composition. Across 47 reporting windows, cadence
averaged **59.77 fields/s**, ranging **59.42-59.85**.
No unexpected resets, panics or idle recovery yields were observed.

Mean render times from each first-cycle stage, excluding the first reporting
window after its switch/startup, were:

| Stage | Render time |
| --- | --- |
| AFTER HOURS / ALL LAYERS | 9.60 ms |
| UNLIT GEOMETRY | 4.73 ms |
| MIRRORS + ALPHA FLOOR | 6.60 ms |
| ADDITIVE LIGHT MESHES | 9.30 ms |
| SPRITE HALOS | 9.59 ms |

These are scene averages across camera movement, not locked-pose microbenchmarks.
Full-scene accepted triangle counts were roughly 516-552 in the capture, including
mirrored geometry and light meshes. The unlit stage was about 187-200 triangles.
TRI/S divides accepted triangles by actual render time. Halos and caption sprites
are composited at full LCD resolution during scanout; their cost is included in
cadence/scanout, not render MS. Scanout was approximately 16.62-16.76 ms.

Startup memory: **15,251 bytes internal** and **8,337,900 bytes PSRAM** free.
Mirror copies consume mesh storage and raster time; there is no depth buffer or
additional reflection image buffer. The sign and two halo quarters total 13 KiB
in flash, with separate immutable label data. Firmware is **0x7bc20 bytes**,
leaving 52% of the 1 MiB app partition free. The board's existing 16 MiB physical
versus 8 MiB configured flash warning is unchanged.

The initial scene and subsequent closer camera were approved on the S3. P4 defaults are provided but were not tested on hardware.

## Closer camera revision

The camera now moves across X +/-360 (previously +/-210), rises and falls
through Y 415-545, and moves between Z -1190 and -1010. The 14-second
cycle retains a controlled forward-facing arc. Native checks passed with eight
serial/parallel comparisons, including four quarter-cycle camera views; the
resulting montage was inspected for framing and floor coverage.

Rebuilt and flashed on S3. A 22-second serial capture covers the full camera
cycle with all layers, followed by the bare stage. The 13 complete-scene reporting
windows averaged 58.90 fields/s, ranging 57.90-59.42,
with render time 10.02-11.45 ms. The larger on-screen scene slightly
increases fill/scanout cost relative to the previous camera. No panics, unexpected
resets or idle recovery were observed. Firmware size is 0x7bc30 bytes.
The user approved the closer camera and scene on the S3.
