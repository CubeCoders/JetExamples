# Neon car validation - 2026-09-23

ESP32-S3 revision 0.2, 240 MHz, octal 8 MiB PSRAM at 80 MHz, 480x320 output,
half-width alternating fields, shared dual-core renderer and queued SPI scanout.

## Correctness

- Five native tests pass: car loading/rendering, OBJ/MTL behaviour, environment
  reflection coordinates, unlit span coverage and constant-normal lighting.
- The car test checks all 404 imported triangles and six materials, four orbit
  views, both field parities, guard pixels, changing reflection coordinates and
  32 parallel-band comparisons, including odd split boundaries. Parallel output
  matches serial output exactly.
- Loader tests cover RGB565 colour/alpha, named texture binding (including spaces),
  missing texture libraries/maps, `mtllib`, index validation, quad triangulation,
  vertex sharing, UV seams, normals and scale.
- Environment tests check reflection directions, translated coordinates, normal
  magnitude, zero-input fallback, panorama seams and a full orbit.
- Unlit coverage is checked against independent edge equations for 600 generated
  triangles, both parities and off-screen clipping, preserving the renderer's
  existing even-aligned bounding convention. Guard pixels remain intact.
- Before subsequent scene edits, the optimised span path and disabled reference
  path produced byte-identical four-view car previews.
- Existing lighting (eight), crate (two), island (three) and depth comparison
  (six) tests pass against the updated Jet dependency.

Native previews confirm readable livery after the model handedness conversion.
The source texture atlas retains its intended wheel/body regions. The revised
dark workshop, orientation correction and performance changes are flashed on S3;
final hardware visual feedback is pending. The initial car version was approved.

## Constant-normal lighting shortcut

The renderer detects exact equality of all three transformed/clipped normals.
Phong evaluates its normalized normal, diffuse response and additive gloss once
per triangle. Gouraud shares one brightness value and skips interpolation only
when its cached vertex values also agree. This is valid for Jet's current
directional light and fixed view-vector lighting; position-dependent per-pixel
lights or view vectors would need separate eligibility rules.

- 48 analytic Phong/Gouraud cases pass, covering textures, both field parities,
  gloss enabled/disabled, non-unit and zero normals, back-facing normals, cached
  lighting and framebuffer guards. An unequal-cache case retains its gradient.
- Fourteen fallback cases were linked against the previous and updated Jet
  libraries. They include a one-unit difference in each normal component and
  equal normals with different cached Gouraud values. All 14,336 pixel samples
  are byte-identical.
- In the four-view car preview, 5,164 of 614,400 pixels changed from avoiding
  intermediate fixed-point normal interpolation/rounding. Maximum 8-bit channel
  difference is 16; mean absolute channel difference is 0.0155. The appearance
  and gloss are retained, but this is not a bit-exact transformation.
- The prior 19 showcase regression tests also pass. The optimised car was built,
  flashed and measured for a full orbit on S3; visual feedback is pending.

## S3 timing and memory

Serial captures span more than one complete 20-second orbit. Report windows
sample moving viewpoints, so these ranges are workload measurements rather than
matched-pose microbenchmarks.

| Version | Render time | Field FPS |
| --- | ---: | ---: |
| Initial car and general raster path | 35.21-39.60 ms | 25.04-28.12 |
| Dark room and fast unlit spans | 19.57-23.33 ms | 42.36-50.39 |
| Corrected handedness and static mesh memory placement | 18.91-22.77 ms | 43.41-52.14 |
| Automatic constant-normal lighting, including first window | 16.82-19.89 ms | 49.64-58.47 |

Excluding the first reporting window, the new capture averages 55.50 fields/s
and 17.75 ms render time, versus 48.23 fields/s and 20.47 ms previously (about
15% higher cadence and 13% less render time). These separate moving-orbit captures
are not matched-pose microbenchmarks. Settled new cadence ranges 51.36-58.47.

The final version spends about 5.5-5.7 ms in scene setup, down from 6.2 ms before
the mesh-placement change. Scanout takes 16.34-16.36 ms. No recovery yields or
unexpected resets were seen. The S3 reports 74,211 bytes free internal memory
and 8,242,504 bytes free PSRAM
at startup (before any subsequent lazy allocations). No depth buffer is allocated.

Firmware size is 0x75d70 bytes; 54% of the 1 MiB application partition remains
free. P4 has not been built or hardware-tested for this example.


## Texture filtering measurement

A temporary S3 build compared the current 256x256 indexed livery with an exact
palette expansion to RGB565, then enabled bilinear filtering on that RGB565
texture. All three modes repeated the same 20-second camera path in one binary.
Window environment filtering remained enabled throughout. Both texture formats
were read from flash; no texture copy to DRAM was introduced.

The first two serial reporting windows after each transition were discarded.
These are arithmetic means of the remaining moving-scene windows (16, 16 and
14 respectively), not a fixed-pose or sampler-only benchmark:

| Livery | Mean field FPS | Mean render time |
| --- | ---: | ---: |
| Indexed nearest (current) | 56.06 | 17.57 ms |
| RGB565 nearest | 53.89 | 18.28 ms |
| RGB565 bilinear | 50.17 | 19.67 ms |

Filtering adds about 7.6% render time and reduces cadence by 6.9% versus RGB565
nearest. Including the format change needed by the current filtering path,
render time increases 11.9% and cadence drops 10.5% versus the indexed livery.
Bilinear cadence ranges 46.57-52.85 fields/s across the measured views.

The RGB565 livery occupies 128 KiB instead of 64 KiB of indices plus a 512-byte
palette. The temporary benchmark held both formats; a permanent conversion would
only need the RGB565 one. The original indexed demo source and firmware were
restored after measurement; no benchmark mode was added to the published example.


## Nearest-sampled windows (current configuration)

The windows now use `environment.bilinear=false`. Both livery and environment
textures use nearest sampling, so `BILINEAR_FILTER=0` also removes the unused
filtering code from this example. Textures, geometry, orbit and Phong settings
otherwise remain unchanged. Five native car/engine checks pass with this build.

The S3 was built, flashed and measured over a full orbit. Omitting the initial
reporting window, 19 windows average **55.92 fields/s** and **17.61 ms**
render time; cadence ranges **52.14-58.77 fields/s**.
Compared with the earlier normal-orbit run using bilinear windows (55.50 average),
the gain is small and does not achieve a sustained 60. These separate orbit
captures are approximate comparisons rather than matched-frame microbenchmarks.

Scanout remains 16.34-16.35 ms. No recovery yields or unexpected resets were seen.
Startup free internal memory is 74,211 bytes; free PSRAM is 8,242,504 bytes.
Current firmware size is 0x75c60 bytes. The nearest-window version remains on S3
for visual review. Historical filtering measurements above used bilinear windows.


## Unlit wheels (current configuration)

All four wheel meshes now use textured UNLIT materials, with specular strength
and exponent set to zero. The original OBJ's first wheel shares the body material;
asset preparation rebinds only that object's `usemtl` to the existing identical
second-wheel material. The original OBJ/MTL/image files remain unchanged.
The combined mesh retains its painter ordering, geometry and UVs.

Five native tests pass. The car preview additionally verifies exactly 200 Phong
body triangles and 200 unlit wheel triangles, all with the same livery texture;
window geometry, guard checks and parallel-band comparisons continue to pass.

Built, flashed and measured over a full S3 orbit. After excluding the initial
window, 20 reporting windows average **56.69 fields/s** and **17.36 ms**
render time, with a range of **53.38-59.25 fields/s**. The preceding lit-wheel
version averaged 55.92 fields/s. This is a modest gain and still falls short of
sustained 60; separate moving-orbit captures are approximate comparisons.

Scanout is 16.34-16.36 ms, with no recovery yields or unexpected resets observed.
Startup internal/PSRAM free memory remains 74,211 / 8,242,504 bytes. Firmware size
is 0x75cd0 bytes. This version remains running on S3 for visual review.


## Coarser grid and rectangular shadow (current configuration)

Grid spacing doubles from 220 to 440 units while retaining its footprint and line
width: 34 quads / 68 triangles become 18 quads / 36 triangles. The oval shadow
becomes one rectangular quad / two triangles with the same 470-by-1100 bounds.
Its former 24-segment fan stored 48 triangles, including 24 degenerates; this
change removes 78 stored triangles overall (54 non-degenerate source triangles).
Both surfaces remain unlit and untextured, with no depth buffer.

Five native checks pass, including four orbit previews, buffer guards and 32
parallel-band comparisons. The build was flashed to S3 and captured over a full
orbit. Excluding the initial window, 21 windows average **59.79 fields/s**,
**15.61 ms** render time and **4.80 ms** setup time. Cadence ranges
**58.73-59.95 fields/s**; render time ranges
14.49-16.70 ms. This is close to the 60-field pacing limit,
with small dips rather than a guarantee of a locked 60. The previous scene
averaged 56.69 fields/s and 17.36 ms render time in a separate orbit capture.

Scanout remains 16.33-16.35 ms. No recovery yields or unexpected resets were seen.
Startup internal/PSRAM free memory is 74,031 / 8,255,844 bytes. Firmware size is
0x75c10 bytes. The coarser grid and rectangular shadow remain on S3 for review.


## Inset shadow adjustment

The rectangle is reduced to 360 by 930 units, centred beneath the car's roughly
398-by-1019 footprint. This leaves about 19 units per side and 44 units at each
end. Height, colour and two-triangle topology remain unchanged.

Five native checks pass and four orbit views were inspected. The S3 build was
flashed and a short startup/render capture checked. The earlier full-orbit timing
above belongs to the larger rectangle; no new full-orbit benchmark was needed
for this size adjustment.


## Brighter scene palette (current configuration)

Raised the unlit floor, walls, platform, ribs, grid and background colours while
retaining the cyan/magenta neon accents. Body ambient lighting rises from
RGB (95, 87, 120) to (130, 123, 150); directional lighting and Phong specular
settings are unchanged. The generated window panorama also has brighter dark
tones. The inset shadow remains dark enough to separate it from the platform.

Five native checks pass and four orbit previews were inspected. Built and flashed
to the S3, then captured over a full orbit. Excluding startup, 21 windows
average **59.77 fields/s** and **15.65 ms** render time, with cadence ranging
**58.68-59.95 fields/s**. No recovery yields or unexpected
resets were observed. Firmware size is 0x75c20 bytes; startup memory remains
74,031 bytes internal and 8,255,844 bytes PSRAM free. The brighter version is
running on S3 for display-level visual feedback.
