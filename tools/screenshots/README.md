# ESP32-style documentation screenshots

These tools build each example against **its own firmware JetConfig.hpp** using
a native C++17 compiler. No ESP-IDF installation or attached board is needed for
the captures; ESP-IDF is required to build the actual firmware.

From the repository root, with CMake, a C++17 compiler and Python with Pillow:

```sh
cmake -S tools/screenshots -B build-screenshots -DCMAKE_BUILD_TYPE=Release
cmake --build build-screenshots --config Release
python tools/screenshots/render.py
```

Use a developer prompt for MSVC. If needed, install the image-conversion package
with `python -m pip install Pillow`. It is only needed for PNG conversion, not
for the firmware or C++ renderer. The native executable also works directly:

```sh
build-screenshots/esp32-neon-film 8 waterfront.ppm
```

On Windows the executable has an `.exe` suffix; Visual Studio generators place
it inside `build-screenshots/Release`. `render.py` accepts `--build-dir` and
`--output-dir`, and handles both single- and multi-configuration build layouts.
The checked-in [manifest](../../docs/screenshots/manifest.json) selects the poses.

## Capture fidelity

- 480×320 output, RGB565 colour and the S3's half-width field buffers.
- Each project's firmware lighting, texturing, depth and post-effect switches.
- The timeline advances at a fixed 60 Hz for deterministic simulation. This is
  a pose-selection clock, not a hardware frame-rate measurement.
- Both field parities are rendered at the selected frozen pose to avoid motion
  combing in the still image. Previous-field reflection buffers are settled.
- Sprites, scene captions and post effects are retained. The runtime performance
  overlay is never attached, so FPS/MS/TRIS/TRI/S and its background are absent.
- No supersampling, desktop quality configuration, smoothing, colour grading,
  artificial glow enhancement or image resizing is applied.
- REPEAT's mesh-storage caption uses the verified 32-bit S3 payload layout
  instead of the larger pointer-containing Triangle records of a 64-bit host.

These are native software captures, not photographs or readbacks from the LCD.
Panel colour response, tearing and hardware timing are not simulated. The
generated PPMs are losslessly converted to PNG; the manifest records image hashes
and the renderer/configuration revisions (configuration hashes use LF line endings).
Buffer guards are checked while
capturing. `PerformanceOverlay` remains enabled in the actual firmware.
