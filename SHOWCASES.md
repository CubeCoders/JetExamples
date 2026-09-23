# S3 showcase series

Each row becomes an independent ESP-IDF project using the shared ESP32 runtime.
Build, flash, measure and review one example on S3 before starting the next.
The cube remains the minimal reusable template.

| Order | Project | Features to demonstrate | Status |
| --- | --- | --- | --- |
| 01 | `esp32-lighting-teapot` | Authentic Utah teapot, smooth normals, Flat/Gouraud/Phong cycle, ambient/directional lighting, diffuse/specular material controls, additive gloss, FPS/TRIS/TRI/S overlay | Approved on S3 |
| 02 | `esp32-textured-boxes` | Large crate cube; affine/perspective mapping and nearest/bilinear cycle | Approved on S3 |
| 02b | `esp32-texture-features` | Wrap/clamp/zero addressing, keyed textures, animated palettes, texture LOD | Planned |
| 03 | `esp32-tropical-island` | Game-style sky gradient, beach and palm geometry, orbit and distance variation, painter sorting, rippled water, previous-field reflections, additive sun/lens-flare sprites and picking-based occlusion | Approved on S3 |
| 03b | `esp32-depth-teapot` | Fixed glossy Phong; seven-second painter/depth cycle on matched motion, overlap artefacts and timing/memory tradeoff | Approved on S3 |
| 03c | `esp32-neon-car` | Runtime OBJ/MTL loading, indexed livery, textured glossy Phong, view-dependent bilinear environment mapping on separate windows, dark neon workshop | Initial appearance approved; dark/orientation/performance revision on S3 for review |
| 04 | `esp32-shading-gallery` | Flat, Gouraud, Phong, unlit and wireframe comparisons; per-face materials; coloured ambient and depth brightness | Planned |
| 05 | `esp32-postfx-crt` | Zero-extra-buffer CRT scanlines, using a scene with readable fine detail | Planned |
| 06 | `esp32-postfx-cel` | Zero-extra-buffer quantised lighting/cel shading, compared with smooth shading | Planned |
| 07 | `esp32-particles` | Fixed particle pools, sparks, water splashes, lifetime, gravity, distance culling, additive effects | Planned |
| 08 | `esp32-sprites-blending` | Full-resolution overlays, colour keys, combined alpha, additive blending, scaling, flips/mirrored quarters, z-order, fades and letterboxing; 3D blend modes | Planned |
| 09 | `esp32-geometry-depth` | Primitives, billboards, near/far clipping, culling, depth bias, optional Z buffering, picking and distance/LOD fades | Planned |

Every completed project will document its configuration, assets, measured S3
cadence and compromises. Relevant showcase assets stay with their example;
portable reusable helpers can be shared. Buffered full-screen effects and
desktop checkerboard reconstruction are not zero-memory S3 paths: assess them
separately instead of implying the existing field runtime supports them.

Feedback on the currently flashed example gates the next one. This list tracks
the intended coverage; a planned row is not an implemented demo.
