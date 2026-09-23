# Texture Lab - addressing, transparency, palettes and texture LOD

A labelled console cycles through seven stages, seven seconds each (49 seconds
per loop), using the shared ESP32 scanout and FPS/MS/TRIS/TRI-S overlay.

| Stage | What to watch |
| --- | --- |
| WRAP | The arrow tile repeats as the same UV rectangle scrolls beyond the image edges. |
| CLAMP | The same animation stretches the coloured edge texels instead. |
| ZERO + black key | Out-of-range UVs return black; black is keyed transparent, revealing the checker behind the panel. |
| Colour key | Two panels share one palm image. The left retains its magenta background; the right keys it out. |
| Palette cycling | A fixed 64x64 index image animates by shifting a 64-colour lava palette lookup. |
| LOD OFF | The panel moves away and back; its texture stays enabled throughout. |
| LOD ON | The exact same distance sweep fades the texture into a solid material colour, then restores it. |

Texture LOD is **not mipmapping or mesh LOD**. Jet's `textureLodNear=1100` and
`textureLodFar=1650` control a per-triangle depth-based crossfade to the material
colour. The panel travels from Z=850 to Z=1900 and back. At the far endpoint the
unlit fast path fills solid spans without per-pixel texture sampling. The fade
band still samples and blends the texture. Changes in panel screen area also
affect render time; compare corresponding points in the two matched sweeps.

ZERO addressing alone returns colour zero; transparency requires `hasAlpha=true`
and `alphaColor=0`. The palm uses magenta (`0xf81f`) as its key. These are hard
cut-outs, not smoothly blended alpha edges.

Palette pixels and colour entries remain immutable in flash. Only `paletteOffset`
changes, at 18 steps/second based on absolute stage time, so animation does not
depend on frame-rate rounding. The lava needs 4 KiB of indices plus 128 bytes of
palette, with no rewritten pixel buffer or extra animation frames.

## Rendering configuration

All geometry is unlit and nearest-sampled. Textured panels face the camera and
each has constant depth across its surface, so affine UV interpolation is exact.
Perspective correction, bilinear filtering and lighting are compiled out here;
their appearance/performance comparisons live in the preceding examples.

Painter rendering uses no depth buffer. The console/backdrop occupy the background
band and the demonstration cards the foreground band, keeping receding LOD panels
visible within the console. This is a deliberately layered presentation, not a
general arbitrary-geometry occlusion scheme. The two keyed cards never overlap.

The HUD retains full panel resolution over half-width alternating fields. FPS
counts displayed fields; MS includes scene setup and both raster workers; TRI/S
uses actual render time rather than the 60 Hz pacing interval. Values can briefly
span two stages during a switch. See [validation](VALIDATION.md).

## Build and validate

From an ESP-IDF 6.0.x terminal in this directory:

```sh
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" build
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" -p PORT flash monitor
```

Use the [reference wiring](../esp32-template-cube/README.md). P4 defaults are
included but this example is validated on S3 only.

With CMake and a C++17 compiler (use a developer prompt for MSVC):

```sh
cmake -S tests -B build-preview -DCMAKE_BUILD_TYPE=Release
cmake --build build-preview --config Release
ctest --test-dir build-preview -C Release --output-on-failure
```

The test emits `texture-lab.ppm` with eight views, including both fading and flat
LOD states. Preview counters remain unmeasured; hardware supplies timing figures.
Generated asset/label headers are checked in. Regenerate them with Pillow and
`python tools/prepare_assets.py`, optionally passing `--font path/to/font.ttf`.
