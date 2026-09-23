# Neon Motorworks - imported model and environment mapping

A 20-second camera orbit around a glossy race car in a dark neon workshop.
The supplied OBJ and MTL are parsed by Jet at startup: this is a model-loading
example, not a generated C++ mesh. The car contains 404 triangles, including
four triangles for its separately authored front and rear windows.

## Rendering features

- Textured Phong paint with a coloured ambient light, directional light and
  additive specular highlights (exponent 32). This creates a metallic appearance;
  it is not a physically based metal/roughness material.
- A shared 256x256, 256-colour livery: 64 KiB of indices plus a 512-byte RGB565
  palette. Palette textures use nearest sampling and perspective-correct UVs.
- A 128x64 RGB565 workshop panorama with bilinear filtering on the windows.
  `environmentReflectionUV` computes view-dependent reflection coordinates from
  world-space surface positions, normals and camera position. U is unwrapped
  across the panorama seam before triangle interpolation.
- Opaque chrome-style windows reflect the distant-room panorama. This is an
  approximation: it does not capture the current framebuffer, reflect the car
  itself, or reproduce nearby geometry with parallax.
- Unlit workshop geometry, neon strips and a baked contact-shadow shape, with
  painter sorting and no depth allocation. Background geometry is submitted
  before the sorted body and windows. This arrangement is tailored to the orbit;
  it is not a general solution for arbitrary intersecting models.

The original export's inward normals and mirrored coordinate system are
converted in `loadCar`: reflect positions across X and negate the Y/Z normal
components, retaining the original triangle order and UV islands. The texture
conversion also changes the image's vertical origin to match OBJ UVs. Flipping
the whole atlas horizontally would put tyre and body regions on the wrong faces.

Jet's loader reuses vertices with identical position/UV/normal indices, retaining
UV and hard-normal seams. MTL textures are matched against a supplied texture
library; unresolved maps retain the material colour. The loader currently expects
positive `v/vt/vn` face indices and triangles or quads. It is not a full OBJ/MTL
implementation or an image decoder.

## ESP32 performance

The shared runtime provides the same dual-core rendering, queued scanout and
full-resolution FPS/MS/TRIS/TRI-S overlay as the other examples. FPS counts fields;
MS and TRI/S use actual render time, including setup and both raster workers.

Jet's opaque untextured UNLIT span path makes the room cheap even in a build with
Phong and texturing enabled. Eligible spans use paired/SIMD RGB565 stores while
textured, lit, blended and custom-shader materials retain their regular paths.
The example places larger static mesh allocations in PSRAM during startup,
then restores the normal allocation threshold, leaving internal memory for hot
transform scratch. Immutable texture data stays in flash.

Jet also detects exactly matching triangle normals in Phong/Gouraud and evaluates
their lighting once per triangle. Phong keeps its additive specular highlight;
Gouraud also requires matching cached vertex brightness. This model's 400 body
and wheel triangles have constant normals, so all are eligible without changing
materials. Smooth triangles keep their interpolating path. Fixed-point colour
rounding can differ slightly from the original per-pixel normal interpolation.

Measured S3 cadence is approximately 51-58 fields/s after startup, up from 45-52
before the constant-normal shortcut and 25-28 before the earlier optimisations. See [the validation record](VALIDATION.md) for conditions and limits.

## Build and preview

From an ESP-IDF 6.0.x terminal in this directory:

```sh
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" build
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" -p PORT flash monitor
```

Use the [template wiring](../esp32-template-cube/README.md). P4 configuration is
included, but this example has only been built and measured on S3.

For a C++17 host compiler and CMake (use a developer prompt for MSVC):

```sh
cmake -S tests -B build-preview -DCMAKE_BUILD_TYPE=Release
cmake --build build-preview --config Release
ctest --test-dir build-preview -C Release --output-on-failure
```

The tests write `car.ppm`, showing four orbit views. Preview counters remain
unmeasured. Hardware provides the performance measurements.

The source files and prepared textures live in `assets`; generated headers are
checked in, so Python is not required to build the firmware. To regenerate them,
install Pillow and run `python tools/prepare_assets.py`. Optionally pass
`--font path/to/font.ttf` for the two bitmap labels. See [asset provenance](assets/README.md).
