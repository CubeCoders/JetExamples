# Particle Lab

A small unlit chamber demonstrates Jet's fixed-pool particle system through four
seven-second stages (28 seconds per loop):

| Stage | What to watch |
| --- | --- |
| Additive sparks | White-hot bursts cool to electric blue and fade; overlapping sparks add light. |
| Water spray | The nozzle sweeps its inherited velocity around a cone; droplets use ordinary alpha-over and their own foam-to-blue lifecycle. |
| Pool limit | Each spark burst requests 260 particles from the shared 200-slot pool. The cap holds and dead slots are reused; emission can be dropped while full. |
| Distance cull | The camera pulls back beyond the 1500-unit particle LOD distance, then returns. LIVE remains nonzero while DRAWN drops to zero. |

Particles use the engine's `emitSparks`, `emitWaterSplash`, `update` and `render`
methods. One static 200-element pool occupies 7,200 bytes on S3, with no heap
allocation per emission. Semi-implicit gravity/velocity updates and emissions
run at 120 Hz using an accumulator. Catch-up is limited to 0.1 seconds after a
long stall. Lifetimes, colours, fades and velocity-aligned triangular streaks
come from Jet. This is a lightweight spray effect, not a fluid simulation.

`ParticleSystem::additiveSparks=true` opts into additive sparks. Its default is
false, preserving existing users' alpha-over rendering. Droplets retain their
alpha-over path regardless of that setting. `activeCount()` counts occupied
slots; `lastRenderedTriangles` counts triangles accepted by the last draw.
Some live particles can be skipped for distance, projection, lifetime fade or
degenerate coverage. No particle sorting or depth buffer is used here: effects
are foreground overlays after the opaque chamber, whose geometry stays behind
their intended flight paths.

## Counters and runtime integration

FPS, MS, TRIS and TRI/S remain visible, alongside particle LIVE and DRAWN counts.
TRIS includes both chamber geometry and accepted particle triangles. MS and
TRI/S include both scene rendering and the particle draw; simulation/update time
remains excluded, consistently with other examples.

The shared runtime's optional fourth `start` argument is a `RenderEffects`
callback. It runs on core 1 after `Scene::render` has joined its raster workers,
before the render timer stops and before the field is published. It returns the
additional accepted triangle count. It must append to the current field without
clearing, swapping or advancing the scene, and must not change texture pixels
borrowed by concurrent scanout. Existing examples omit this callback and retain
their existing path. The older `afterRender` callback remains outside the timer.

Particle drawing is serial; the chamber still uses the two raster workers.
The serial log's historical `setup` label includes this additional draw time,
while `raster` measures the chamber's raster workers only.

All numeric label textures are immutable. Updating their material pointers lets
the normal scanout snapshot protect the visible counters without rewriting an
image that the other core is reading. There are no scene texture assets; the
chamber is generated geometry and the overlay has generated text bitmaps.

## Build and check

From an ESP-IDF 6.0.x terminal in this directory:

```sh
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" build
idf.py -B build-s3 "-DIDF_TARGET=esp32s3" "-DSDKCONFIG=sdkconfig.s3" -p PORT flash monitor
```

Use the [reference wiring](../esp32-template-cube/README.md). P4 defaults are
included; S3 is the hardware validation target.

With CMake and a C++17 compiler (developer prompt for MSVC):

```sh
cmake -S tests -B build-preview -DCMAKE_BUILD_TYPE=Release
cmake --build build-preview --config Release
ctest --test-dir build-preview -C Release --output-on-failure
```

The tests cover pool exhaustion/reuse, gravity, life expiry, sparks versus water
blending, field parity, projection/distance/fade culling, counters and guards.
The integration test advances an entire cycle, reaches all 200 slots, verifies
live-but-culled particles and compares serial/parallel geometry plus particles.
`particles.ppm` contains six views; its performance counters are placeholders.
Random particle positions can vary with the platform's C-library RNG.

Regenerate label headers with Pillow and `python tools/prepare_labels.py`,
optionally passing `--font path/to/font.ttf`. The generator uses a locally
installed monospace font; the font file is not distributed.
See [hardware validation](VALIDATION.md) for measurements.
