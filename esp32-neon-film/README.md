# ION — neon city film

A 101-second, eleven-shot Jet cinematic, followed by a one-second black hold
and an intentional ESP32 restart. This example is being reviewed and tuned;
the 50–60 fields/s performance target is not yet met throughout.

The original low wedge coupe, compact streets and framing follow the
[car design](references/car-design.png) and one concept reference per shot.
These generated references guide composition and proportions; they are not
screenshots or promises of ESP32 image quality. The former NASCAR model is
not used or bundled in this project.

| Time | Shot | Reference |
| --- | --- | --- |
| 0–12 | River skyline, fade-in, searching sky beams, floating holograms | [01](references/cut-01.png) |
| 12–22 | Descending camera in a rainy narrow street; mirrored shopfronts | [02](references/cut-02.png) |
| 22–29 | Oblique overhead courier and tyre spray | [03](references/cut-03.png) |
| 29–38 | Close three-quarter orbit around the front of the car | [04](references/cut-04.png) |
| 38–47 | Angled driver-height cockpit, bonnet and red scanner | [05](references/cut-05.png) |
| 47–53 | Oblique instrument close-up: 70 to 88 MPH | [06](references/cut-06.png) |
| 53–64 | Right turn into a compact three-lane boulevard | [07](references/cut-07.png) |
| 64–78 | Staged overtakes, pursuing police and four camera positions | [08](references/cut-08.png) |
| 78–88 | Wheels slow and hinge downward; cyan hub glow builds | [09](references/cut-09.png) |
| 88–93 | Pitched climb, forward flight and frozen police | [10](references/cut-10.png) |
| 93–101 | Ascending fly-by, city overview and fade-out | [11](references/cut-11.png) |

Camera translation is linear within each tracking shot; the front orbit uses
constant angular progression. Smooth lane changes describe vehicle steering,
not camera easing. The wheels provide the entire hover conversion: no wings
or deployed rear engine. The launch has no flash: the car retains its forward
speed of 88 MPH while climbing at 650 world units/s. Its nose follows the
flight direction, including the hinged wheels, window reflections and glow sprites. The final camera
waits ahead of its flight path, letting the coupe pass close to the lens before
tilting down over the city and fading out. Road sections recycle around the tracking camera. The
framing uses offset subjects and diagonal street lines, with a fixed lens per
cut rather than a uniform centred composition.

The 570-unit coupe represents a 4.5-metre car: 70 MPH is 6.95 car lengths
per second and 88 MPH is 8.74. Road parallax and wheel rotation use the same
conversion. The tight right turn slows to roughly 25 MPH, then cuts back to
the faster boulevard tracking rig. The speedometer keeps its 70–88 MPH climb.

Street draw distance is 9,000 world units. Recycled streets provide sixteen
blocks and put their longer extent in the viewing direction, including behind
the car for front tracking. The fixed rain street uses simpler distant facades;
the boulevard exit recycles sixteen blocks along its eastbound axis.
Painter sorting uses 128 depth buckets to preserve nearby surface ordering at
this range: still one byte per triangle, plus 512 bytes of sorting stack versus
the default 64 buckets. Jet's default configuration remains unchanged.

At 3,200 units, grouped storefronts switch to six-triangle alternatives that
retain their shop artwork and facade planes. Four box-filtered 32×32 facade
mipmaps occupy 8 KiB of internal DRAM; the cache is initialized once and remains
immutable during rendering. Distant lamp meshes are culled separately.

The colour grade increases contrast by 25% around an 80/255 pivot, deepening
shadows and lifting highlights. It is applied to authored material colours and
baked texture RGB565 values, with no per-pixel postprocess. Additive glow textures
retain their original falloff.

## Rendering and memory

- Shared ESP32 runtime: 480×320 output, half-width RGB565, alternating fields,
  overlapped DMA scanout and two raster workers on the S3. At 60 fields/s an
  individual physical LCD row refreshes at 30 Hz.
- Painter sorting without a depth allocation. Opaque Phong body, unlit tyres
  and scenery, nearest environment mapping on the separately modelled canopy.
- Previous-field river reflection; inverted mesh reflections under the rainy
  street; translucent additive beams and full-resolution sprite halos.
- Bevelled/sloping skyline, shop textures, sloped awnings, projecting signs,
  spinning wheels, flickering lamps and fine low-opacity rain streaks.
- Scene-local geometry/materials are owned by a bank and released at cuts.
  Persistent immutable textures remain valid for concurrent scanout snapshots.
  Large scene allocations prefer PSRAM; internal RAM is reserved for live
  transforms and runtime work. Adjacent shop details share transform work.
- Closed hero body, shared canopy/glass boundaries, inner wheel-well walls,
  a continuous chassis floor and capped wheels that remain sealed in hover mode.
- Enclosed cockpit with sloping windscreen pillars, roof header and door trim.
  One low dashboard holds two flush displays and a slim red scanner; its open
  centre makes the autonomous layout clear. The driver camera stays at road height.
  The speed display updates a scene-local 16 KiB RGB565 texture only when its
  integer value changes; the allocation prefers PSRAM on the S3.
- Only the rounded FPS number appears at the absolute top-right on hardware.
  Full timing and triangle information remains in serial diagnostics.

Assets are original procedural geometry and artwork in `main/Vehicle.hpp`,
`main/City.hpp` and `tools/prepare_assets.py`. Bitmap concept art was made with
the built-in image generation tool; the exact prompts are in
[references/prompts.json](references/prompts.json). The small firmware artwork
is generated with Pillow; rebuilding does not need image generation services.

## Build and review

Initialize the shared submodules from the repository root, then use an ESP-IDF
6.0.1 environment in this directory:

```sh
idf.py -B build-s3 -DIDF_TARGET=esp32s3 -DSDKCONFIG=sdkconfig.s3 build
idf.py -B build-s3 -DIDF_TARGET=esp32s3 -DSDKCONFIG=sdkconfig.s3 -p COM6 flash monitor
```

P4 configuration is included but this film has only been run on S3 hardware.
The complete sequence must be watched before judging resets: restarting after
the final black hold is intentional; an earlier restart is a fault.

Native validation and video export (C++17, CMake, Python, ffmpeg):

```sh
cmake -S tests -B build-preview -DCMAKE_BUILD_TYPE=Release
cmake --build build-preview --config Release
ctest --test-dir build-preview -C Release --output-on-failure
python tools/render_video.py preview.mp4 --renderer build-preview/film_video.exe
```

Use the executable path generated by your build system; multi-configuration
builds may put it in `build-preview/Release`. `--ffmpeg` accepts a custom encoder
path; `--start` and `--seconds` export an excerpt. The exporter streams RGB24
directly into ffmpeg and writes no individual frame files. Its 60 fps video
reconstructs the same alternating packed fields, including previous-field water
and scanline sprite compositing. It omits the hardware FPS overlay: native
export speed is not an S3 measurement. Silent video is for visual review.

A separate desktop quality target renders full 2880×1920 RGB565 colour and
depth buffers at every frame, with bilinear textures, perspective mapping and
full-detail meshes. The export downsamples to 1920×1280 at 60 fps using Lanczos
filtering for spatial anti-aliasing. It has no packed fields, scanline
reconstruction or hardware counters. Geometry, artwork and camera paths are
shared with the S3 build; it remains an actual Jet render, not a ray-traced remake.

```sh
cmake -S tests/quality -B build-quality -DCMAKE_BUILD_TYPE=Release
cmake --build build-quality --config Release
ctest --test-dir build-quality -C Release --output-on-failure
python tools/render_video.py quality.mp4 --quality
```

For multi-configuration generators, also pass `--renderer` with the executable
under `build-quality/Release`. Both export modes write render and encoding logs
next to the MP4. The quality build enables Jet's optional
`JET_HIGH_PRECISION_UVS`: large projected triangles use full-width edge weights
and double reciprocal depths to prevent texture overflow. The embedded path
keeps the option disabled and pays no added pixel cost.

See [VALIDATION.md](VALIDATION.md) for checks and known limitations.
