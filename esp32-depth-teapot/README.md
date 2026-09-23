# Depth testing: painter versus Z buffer

The same glossy red Utah teapot repeats a seven-second rotation, alternating
between painter sorting and depth testing every seven seconds. Pitch and roll
repeat with yaw, so both modes follow the same motion. Phong lighting, geometry,
camera, colour and gloss remain fixed. This isolates visibility handling from
the shading-mode comparison in [the lighting example](../esp32-lighting-teapot/README.md).

Watch where the spout meets the body: painter sorting can show small wedges
from the wrong surface. Depth testing resolves the overlap, at a cost in render
time and memory. Painter sorting is a good default for scenes that do not need
per-pixel overlap resolution; the crate and island examples use it.

The upper-right overlay shows field FPS, render MS, rasterized TRIS and
render-time TRI/S. Samples immediately after a switch can include both modes.
The render time includes clearing, transformation, sorting and both raster
workers. It excludes scanout waits and pacing. Triangle counts are rasterized
triangles after culling, including fully occluded triangles, not visible pixels.

## Runtime depth capability

`Z_BUFFERING=1`, `FAST_Z=0`, and the opt-in `JET_RUNTIME_DEPTH=1` compile
both depth and painter kernels. Between frames the example calls
`scene.getRenderer()->setDepthTestingEnabled(enabled)` before rendering.
The default state is enabled; ordinary builds leave `JET_RUNTIME_DEPTH=0`
and retain their static renderer without the extra kernel.

When disabled, Jet skips depth clearing, reads and writes, selects back-to-front
painter buckets, and removes per-pixel depth interpolation when no other enabled
feature consumes it. Fog, depth brightness, texturing and picking retain their
depth calculations where compiled in. When enabled, opaque triangles are sorted
front-to-back for early depth rejection; blended faces retain painter ordering.
Near-plane geometry clipping remains active in both modes.

This comparison allocates its 153,600-byte PSRAM depth buffer once and retains
it while disabled, so toggles do not allocate memory. A permanent painter-only
project should set `Z_BUFFERING=0` to remove the allocation and unused code too.
The extra kernel also consumes code/IRAM in this comparison build.

The mesh is shared directly from the lighting example's `TeapotMesh.hpp`.
See its [asset provenance](../esp32-lighting-teapot/assets/README.md).
Keep the repository layout intact; each directory remains its own IDF project.

## Build and validate

From this directory with ESP-IDF 6.0.x:

```sh
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" build
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" -p PORT flash monitor
```

Use the [template wiring](../esp32-template-cube/README.md). P4 defaults are
included; this example has only been built and measured on S3 hardware.

With CMake and a C++17 compiler (a developer prompt for MSVC):

```sh
cmake -S tests -B build-preview -DCMAKE_BUILD_TYPE=Release
cmake --build build-preview --config Release
ctest --test-dir build-preview -C Release --output-on-failure
```

The preview writes `depth-teapot.ppm`: painter on the left, depth on the right.
Its counters remain unmeasured. See [validation](VALIDATION.md) for hardware data.
