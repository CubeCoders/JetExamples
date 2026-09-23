# Woodland / LOD

A tree in a daylight meadow changes from a full mesh to a simpler mesh and
finally a camera-facing image as the camera retreats. A path, fence and distant
tree line give the scene scale. The featured tree always occupies the same
world position; the billboard is real world-space geometry, not a Sprite2D.

| Distance from the tree centre | Representation | Source triangles |
| --- | --- | --- |
| Below 1800 | Full mesh | 624 |
| 1800-2400 | Simplified mesh | 176 |
| 2400-2600 | Mesh fades out while billboard fades in | 178 |
| 2600 and beyond | Camera-facing billboard | 2 |

The 32-second loop has two matched 16-second camera passes. The first uses
distance LOD; the second forces the full mesh at every distance. Each moves
from about 1250 to 3300 units away and back, with a sideways arc. Captions show
the featured tree's current representation. The usual FPS/MS/TRIS/TRI/S overlay
counts the whole scene, so its total also includes ground, hills, fence and the
six permanently billboarded background trees.

## Engine features

The high-detail object holds the simple mesh in `lodMeshes`. With
`scene.lodScale = 1800`, Jet chooses the mesh by distance. The low-detail object
is not registered for independent rendering. `lodPersist` retains that last
mesh beyond the final mesh level until the tree's distance fade removes it.

`fadeNear` / `fadeFar` remove the tree across 2400-2600 units. A separate
`Primitives::createBillboard` object uses matching `appearNear` / `appearFar`
values. Separate objects are necessary because Jet's mesh-LOD substitution
borrows geometry while retaining the original object's transform and flags;
putting a billboard in `lodMeshes` alone would not enable camera-facing rotation.

The mesh and billboard share the same centre. A conservative symmetric mesh
bounding box keeps the engine's centre-based distance calculation aligned with
the impostor. Beyond the fade range Jet skips the mesh before its vertex and
triangle work. The reference pass disables the mesh LOD/fade and the impostor.

The billboard follows camera yaw while remaining upright; it preserves world
position and perspective size. The background trees all share the same image.
The intersecting canopy clusters use per-pixel depth testing (`Z_BUFFERING=1`,
`FAST_Z=0`). Average triangle-depth sorting cannot resolve their intersecting
surfaces and produced visibly unstable foliage on S3. Opaque triangles are
ordered front to back to reduce overdraw. Face colours are baked, so there is
no per-frame lighting pass. The shared runtime allocates a 150 KiB depth buffer
in PSRAM; the billboard path also retains depth testing for consistent occlusion.

## Image and transition tradeoffs

The generator creates both tree meshes and a matching orthographic front-view
billboard from the high-detail geometry. One colour-keyed RGB565 image is
128x160 (40 KiB in flash), shared by all seven billboard objects. It has no
runtime capture cost and needs no extra framebuffer.

A single-view impostor does not reproduce changing parallax, lighting or every
viewing angle. This is well suited to distant foliage; asymmetrical landmarks
may need multiple baked directions. This example uses an authored modest arc.
The simple mesh changes shape visibly to make its savings clear.

The mesh-to-billboard interval uses ordinary alpha blending with
`SCREEN_DOOR_ALPHA=0`. Both representations are briefly rendered, so it costs
slightly more geometry during the transition and can look translucent. It is
not a complementary dither or a seamless geometric morph. The ground and fence
use an explicit background ordering appropriate to this camera path.

## Build and inspect

From an ESP-IDF 6.0.x terminal in this directory:

```sh
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" build
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" -p PORT flash monitor
```

Use the [template wiring](../esp32-template-cube/README.md). P4 defaults are
provided but hardware verification here is on S3.

With CMake and a C++17 compiler:

```sh
cmake -S tests -B build-preview -DCMAKE_BUILD_TYPE=Release
cmake --build build-preview --config Release
ctest --test-dir build-preview -C Release --output-on-failure
```

Native checks verify actual rasterized triangle reductions at both LOD
boundaries, billboard configuration, the reference path, colour/depth buffer
guards, opaque visibility under reversed triangle order and
six serial/parallel image comparisons covering both field parities.
`woodland.ppm` shows four LOD views and two reference views; preview performance
counters are placeholders.

Regenerate assets with Pillow and `python tools/prepare_assets.py`, optionally
using `--font path/to/font.ttf`. See [assets](assets/README.md) and
[hardware validation](VALIDATION.md).
