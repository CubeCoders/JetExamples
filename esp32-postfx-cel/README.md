# Cel / Teapot

The glossy red Utah teapot alternates between smooth Phong lighting and
four-band cel lighting, **seven seconds per mode**. Both modes repeat the same
complete rotation and rocking motion. Geometry, material, lights, camera,
background and depth testing are fixed.

`POSTFX_CELLSHADING=1` makes lighting quantisation available. Set
`scene.getRenderer()->celShadingEnabled` at runtime; `celShadingBits=6` discards
six low brightness bits, giving diffuse levels 0, 64, 128 and 192. Zero bits
preserves smooth lighting; the control is clamped to eight bits. Despite the
historical POSTFX name, this happens **inside the existing raster pass**, with
no second full-screen pass or additional image/row buffer.

This demonstrates quantised diffuse lighting rather than outlines or posterising
the completed image. Ambient colour and the additive Blinn-Phong gloss remain
continuous. The background and overlay are unaffected. There can consequently
be more than four final RGB565 colours on the teapot. The same expensive Phong
normal interpolation and specular calculation still run: cel shading is a style
comparison, not a shortcut to flat-lighting performance.

The teapot keeps its depth buffer in both modes because painter sorting produces
visible spout/body overlap artefacts, as shown in the depth comparison example.
The 150 KiB depth allocation is for geometry visibility, not the cel effect.
The shared fast ESP32 scanout, pacing, parallel raster and FPS/MS/TRIS/TRI-S HUD
remain unchanged. MS and TRI/S use actual scene render time.

## Build and validate

From an ESP-IDF 6.0.x terminal in this directory:

```sh
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" build
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" -p PORT flash monitor
```

See the [reference wiring](../esp32-template-cube/README.md). P4 defaults are
included but only S3 hardware is validated here.

With CMake and a C++17 compiler (developer prompt for MSVC):

```sh
cmake -S tests -B build-preview -DCMAKE_BUILD_TYPE=Release
cmake --build build-preview --config Release
ctest --test-dir build-preview -C Release --output-on-failure
```

Tests cover exact quantisation, runtime disabling, clamped controls, unlit bypass,
constant-normal lighting shortcuts, 28 matched teapot poses, unchanged depth and
background, transitions, buffer guards and 48 parallel-band comparisons.
`cel-teapot.ppm` contains two matched poses; its counters are placeholders.

The generated teapot mesh and its permissively licensed FreeGLUT source are local
to this example; see [asset provenance](assets/README.md). Regenerate the mesh
with `python tools/generate_teapot.py`. Labels can be regenerated with Pillow and
`python tools/prepare_labels.py`, optionally supplying `--font path/to/font.ttf`.
See [hardware validation](VALIDATION.md) for measurements.
