# Jet examples

Standalone ESP-IDF projects using [Jet](https://github.com/cubecoders/jet).
Each example has its own project, renderer configuration and build directory.
Jet and LovyanGFX are shared Git submodules; the ESP32 runtime is shared source.

```
components/
  Jet/                   Jet submodule
  LovyanGFX/             display driver submodule
  esp32_jet/             reusable ESP32 display, scanout and frame runtime
cmake/                   shared dependency setup
esp32-template-cube/     first standalone ESP-IDF project
tests/                   host checks
```

Start with [esp32-template-cube](esp32-template-cube/README.md). Its only visible
scene is a rotating, six-colour 3D cube on a dark background, with a shared
performance overlay.

After cloning this repository, run `git submodule update --init --recursive`.
Keep the repository layout intact: each example references `../components`.
To make another example, copy `esp32-template-cube`, change its CMake project
name and replace `main/Cube.hpp`. Build each project from its own directory.

The runtime preserves the game firmware's fast paths:

- Half-width RGB565 and alternating half-height fields: 153,600 bytes total.
- Core 1 renders while core 0 scans out the completed previous field.
- S3: eight-row DMA queue with completion notifications, SIMD pixel expansion,
  polling fallback, and an adaptive raster worker using core 0 while DMA waits.
- P4: ping-pong scanlines, explicit cache flushing and direct DMA without the
  driver's intermediate byte-swap copy or redundant data cache flush.
- Absolute 60 Hz field deadlines, measured animation timestep, overrun rebasing
  and idle-task recovery while keeping the task watchdog enabled.
- Stable copies of sprite/material state during parallel scanout; Jet handles
  portable RGB565 scanline compositing, including full-resolution overlays.

This is 60 **fields** per second: each physical LCD row updates at 30 Hz.
All examples display FPS, MS, TRIS and TRI/S in a full-resolution overlay:
- FPS is completed render fields per second, sampled over at least 0.5 seconds.
- MS is mean elapsed `Scene::render` time, including setup and both raster workers,
  excluding animation, sprite callbacks, scanout waits and frame pacing.
- TRIS is the most recently completed field's unique rasterized triangle count
  after culling and clipping. It counts triangles accepted for rasterization,
  including fully depth-occluded triangles; it is not a visible-pixel query.
- TRI/S divides the sum of rasterized triangle counts by the sum of measured render
  times in the sample window. It is render throughput for this workload, not
  display throughput or a maximum geometry benchmark. Triangles spanning both
  raster workers count once.

Serial timing reports describe cadence, rendering and scanout separately.
The cube is deliberately small; it is not a renderer throughput benchmark.

## Showcases

See [the S3 showcase series](SHOWCASES.md) for coverage and review progress.
Implemented: [Utah teapot lighting](esp32-lighting-teapot/README.md) and
[crate texture mapping/filtering](esp32-textured-boxes/README.md), and
[the tropical island](esp32-tropical-island/README.md), and
[depth testing](esp32-depth-teapot/README.md), and
[Neon Motorworks model loading and environment mapping](esp32-neon-car/README.md), and
[Texture Lab addressing, transparency, palettes and LOD](esp32-texture-features/README.md).
Each showcase is reviewed on hardware before the next is started.

Painter sorting is the default for the template cube, textured crate, island, neon car and Texture Lab.
The lighting teapot keeps depth testing because its intersecting surfaces show
visible sorting artefacts. The depth teapot explicitly demonstrates that tradeoff.

## Host checks

With CMake and a C++17 compiler (use a developer prompt for MSVC):

```sh
cmake -S tests -B build-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build-tests --config Release
ctest --test-dir build-tests -C Release --output-on-failure
```

Checks cover fractional frame deadlines, the actual cube over 120 fields,
buffer guard pixels, and the Jet sprite scanline compositor in both byte orders.
The cube test writes `cube.ppm` for inspection. The S3 has also been flashed and
checked over serial at 60.00 fields/s with the DMA queue and both raster cores
active. P4 validation currently covers compilation only. See the
[validation record](esp32-template-cube/VALIDATION.md) for measured results.

## Dependency publication

The initial local review includes a Jet support commit and a local LovyanGFX
optimisation commit. The pinned dependency commits must be reachable from their
`.gitmodules` URLs before this examples repository is published. Publish the Jet
branch upstream; publish the LovyanGFX support branch to an appropriate fork (or
have it merged upstream) and update that submodule URL if using a fork. Then
verify a fresh recursive clone. No dependency or examples changes are pushed
automatically by this project.

Example/runtime code is MIT licensed; dependencies retain their own licences.
