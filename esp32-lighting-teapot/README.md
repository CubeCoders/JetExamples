# 01 - Utah teapot: Flat, Gouraud and Phong

![Utah teapot lighting: ESP32-style screenshot, performance overlay hidden](../docs/screenshots/esp32-lighting-teapot.png)

A red Utah teapot rotates beneath a fixed directional light against a blue
gradient. Every three seconds it switches Flat / Gouraud / Phong, with a matching
caption. The same mesh, light and motion continue across each switch. The shared
overlay shows field FPS, rasterized triangles and effective triangles/second.
Its half-second samples briefly mix modes immediately after a transition.

Flat shades each face, Gouraud interpolates vertex lighting, and Phong
interpolates and normalises normals per pixel. Phong opts into
`material.specularExponent = 32`: additive, light-coloured Blinn-Phong gloss.
The half-vector uses the fixed view direction `(0,0,-1)` and is prepared per
triangle; integer repeated squaring replaces a per-pixel `pow()` call.
An exponent of zero preserves Jet's original broad view-facing highlight.

The authentic Utah teapot has 822 vertices and 1,560 triangles, generated from
Bezier patches with analytic smooth normals. See [asset provenance](assets/README.md).
The brisk tilt and roll reduce pauses caused by integer-degree object angles.

`LIGHTING=1`, `Z_BUFFERING=1` and `FAST_Z=0` give interpolated per-pixel depth for
the intersecting handle, spout and body. Per-object triangle sorting is disabled;
the global queue opts into `JET_DEPTH_SORT_OPAQUE_FRONT_TO_BACK=1` to reject
hidden pixels before lighting. That option keeps blended geometry in a later
far-to-near band. The depth buffer is 153,600 bytes in PSRAM. Transform scratch
prefers internal RAM up to 32 KiB with allocation fallback. Texturing, depth fog
and post-effects are disabled.

Both S3 cores rasterize disjoint colour/depth rows while core 0 services queued
DMA scanout. Half-width field buffers and pacing are shared with the cube.
Caption pixels are immutable, and the performance overlay updates between
completed frames. See [validation](VALIDATION.md) for measured performance.

From an ESP-IDF 6.0.x terminal in this directory:

```sh
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" build
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" -p PORT flash monitor
```

Use the [reference wiring](../esp32-template-cube/README.md). P4 defaults are
included, but this showcase has only been hardware-tested on S3.

Host checks (CMake and C++17):

```sh
cmake -S tests -B build-preview -DCMAKE_BUILD_TYPE=Release
cmake --build build-preview --config Release
ctest --test-dir build-preview -C Release --output-on-failure
```

Eight checks cover four glossy preview poses (`teapot.ppm`), normal lengths,
mode timing, lighting response, counter math, depth addressing, integer square
roots, specular accuracy, blended depth sorting, buffer guards and concurrent
raster equivalence. The final red cycling showcase was approved on the S3.

The shared overlay shows field FPS, mean render MS, TRIS and render-time TRI/S.
TRI/S uses summed triangle counts divided by summed render time, excluding
scanout waits and pacing; MS includes scene setup and both raster workers.

Painter sorting was explicitly compared against depth across 120 animated poses
in all three shading modes. It causes visible wedges where the spout intersects
the body, including with full per-object triangle sorting. This example keeps
depth for that reason. See the separate [depth comparison](../esp32-depth-teapot/README.md)
to inspect the speed/visibility tradeoff with fixed Phong shading.
