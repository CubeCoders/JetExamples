# Woodland validation

Built with ESP-IDF 6.0.1 and flashed to the ESP32-S3 on COM6 on 2026-09-23.
Jet remains at fffbf63. No engine or shared-runtime changes were needed.
P4 defaults are provided but were not hardware tested.

## Native checks

Release CTest passed with assertions enabled. Six images match between serial
and parallel rendering, with both field parities and framebuffer guard pixels.
The actual scene produces 654 triangles close up, 209 in the simple-mesh view,
210 during the mixed transition, and 33 at maximum distance. The corresponding
full-mesh reference poses give 654 and 640 triangles. These are accepted raster
triangles, including the rest of the scene, rather than source mesh counts.

Exact distance probes around the engine's LOD boundaries produced 657, 215,
211 and 37 triangles at 1799, 1801, 2500 and 2601 units. Tests confirm the LOD
chain, the two-triangle camera-facing object and the shared centre. The native
montage was visually inspected; the billboard's vertical UV orientation was
corrected before hardware flashing.

## Initial painter-only S3 capture

A 38-second serial capture covers the 32-second LOD/reference loop and its
restart. Across 36 windows, cadence averaged 59.86 fields/s
(range 59.83-59.91). No panics or idle recovery
were observed.

| Representation | Mean fields/s | Mean render ms | Accepted triangles |
| --- | --- | --- | --- |
| FULL MESH | 59.84 | 9.89 | 632-653 |
| SIMPLE MESH | 59.88 | 6.85 | 197-206 |
| BILLBOARD | 59.90 | 4.30 | 32-44 |
| REFERENCE FULL MESH | 59.84 | 8.35 | 571-648 |

The first reporting window after each representation change is omitted from
these rows. Brief transition intervals may have no complete reporting window.
The full-mesh reference row covers the entire moving camera path; compare
on-screen MS at matching distances rather than treating stage averages as a
locked-pose benchmark. Frame pacing can keep FPS near 60 even when rendering
becomes much cheaper. TRI/S remains actual render throughput, not a count
divided by display frame time.

Startup free memory: 52,959 bytes internal and 8,240,344 bytes PSRAM.
Firmware size: 0x77dc0 bytes. The shared billboard texture occupies 40 KiB
in flash; no extra reflection, capture or depth buffer is allocated.

The user reported unstable canopy overlap in this painter-only version; the
revision below enables per-pixel depth testing.

## Per-pixel depth correction

Physical-display feedback identified unstable overlap between the intersecting
canopy clusters. Enabled Z_BUFFERING=1 and FAST_Z=0 with opaque front-to-back
ordering. Average-depth triangle sorting cannot resolve these intersections.
The shared runtime allocates the 150 KiB depth buffer in PSRAM.

Native tests pass with both colour and depth guards. Six serial/parallel colour
comparisons pass, and the depth buffer is confirmed to receive writes. Five
additional opaque poses reverse the mesh triangle order: 232-480 physical output
pixels differ at quantised depth ties (116-240 half-width samples), within the
512-pixel regression allowance. This checks against large ordering artefacts;
it does not claim bit-identical visibility at shared or near-coplanar edges.
The corrected preview montage was inspected, including the fade interval.

Rebuilt and flashed on S3; a 38-second capture covers the complete comparison
loop. Across 29 windows cadence averaged 49.75 fields/s
(range 36.78-59.76). No panics or idle recovery
were observed. Stage means below omit the first window after every switch.

| Representation | Mean fields/s | Mean render ms |
| --- | --- | --- |
| FULL MESH | 41.07 | 24.25 |
| SIMPLE MESH | 56.11 | 17.66 |
| BILLBOARD | 59.76 | 14.98 |
| REFERENCE FULL MESH | 47.67 | 21.09 |

Startup free memory: 61,151 bytes internal and 8,086,740 bytes PSRAM.
Firmware size: 0x75fa0 bytes.
The corrected version is running on S3 for confirmation of visual stability.
