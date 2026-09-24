# 03 - Tropical island

![Tropical island: ESP32-style screenshot, performance overlay hidden](../docs/screenshots/esp32-tropical-island.png)

A sunny low-poly island with five palms, faceted sand and rocks, turquoise
water, a bright sky gradient and an orbiting camera. The camera circles every
15 seconds, gently approaching and retreating between radii of 1,540 and 2,060
units over a 30-second cycle. The ocean is a 9,600-unit square split
into 36 tiles (72 triangles); the complete scene has 47 objects, 626 triangles
and 608 vertices. Geometry is generated once at startup and cached.

## Engine features

- `WATER_REFLECT` samples the immutable previous field, retaining the game's
  rippled screen-space reflection technique without another framebuffer.
- The engine tracks the horizon from camera pitch. This example projects the
  water plane at the island centre to calculate an additional reflection
  offset from camera height, angle and distance. An optional source-row cutoff
  fades shifted scene reflections into sky to prevent recursive water feedback.
- Reflection ripple strength 30 and blend alpha 160 match the game's active
  BayFable track. Its existing ripple timing and perspective spacing are retained.
  This demo uses a flat sea rather than BayFable's displaced clip-map mesh.
- Painter's sorting (`FAST_Z=1`, `Z_BUFFERING=0`) enables the game's fast spans.
  The ocean occupies the background sort band and all land is above Y=0, so it
  cannot paint over the island. Water retains Z bias -2 if depth testing is enabled.
  No depth buffer is allocated in this configuration.
- Jet's `LensFlare` projects a sun direction and uses a pick query to fade when
  geometry occludes it. Seven additive sprites use mirrored-quarter textures:
  three 16x16 RGB565 resources occupy just 1.5 KiB, generated once at startup.
- The shared runtime overlaps scanout and rendering, with two raster workers,
  field pacing and a full-resolution FPS / MS / TRIS / TRI/S overlay.

Colours are authored per face with unlit materials; this keeps the focus on
water and compositing. The lighting showcase covers dynamic lighting. No
external art, font file or game dependency is required. The scene and flare
generation adapt the game's palm and mirrored-sprite construction techniques.

Reflections are a deliberately inexpensive screen-space approximation: they
only contain geometry visible in the preceding field and use one depth anchor
at the island centre. They are not planar-camera reflections or ray tracing.
The sky and finite ocean meet in a distant colour band; the square boundary is
kept outside the foreground throughout the orbit. This example does not enable
depth fog; that remains available for later scenes.

## Build and validate

From an ESP-IDF 6.0.x terminal in this project:

```sh
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" build
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" -p PORT flash monitor
```

Use the [reference wiring](../esp32-template-cube/README.md). P4 defaults are
included but this scene has only been flashed and measured on the S3.

```sh
cmake -S tests -B build-preview -DCMAKE_BUILD_TYPE=Release
cmake --build build-preview --config Release
ctest --test-dir build-preview -C Release --output-on-failure
```

The preview writes `island.ppm` with four views around the orbit. Its overlay
remains unmeasured; actual timing comes from hardware. See [validation](VALIDATION.md).
