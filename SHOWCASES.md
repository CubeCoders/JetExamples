# S3 showcase series

Each row becomes an independent ESP-IDF project using the shared ESP32 runtime.
Build, flash, measure and review one example on S3 before starting the next.
The cube remains the minimal reusable template.

| Order | Project | Features to demonstrate | Status |
| --- | --- | --- | --- |
| 01 | `esp32-lighting-teapot` | Authentic Utah teapot, smooth normals, Flat/Gouraud/Phong cycle, ambient/directional lighting, diffuse/specular material controls, additive gloss, FPS/TRIS/TRI/S overlay | Approved on S3 |
| 02 | `esp32-textured-boxes` | Large crate cube; affine/perspective mapping and nearest/bilinear cycle | Approved on S3 |
| 02b | `esp32-texture-features` | Seven-stage Texture Lab: wrap/clamp/zero addressing, keyed foliage, indexed lava palette cycling, matched texture-LOD sweeps | Approved on S3 |
| 03 | `esp32-tropical-island` | Game-style sky gradient, beach and palm geometry, orbit and distance variation, painter sorting, rippled water, previous-field reflections, additive sun/lens-flare sprites and picking-based occlusion | Approved on S3 |
| 03b | `esp32-depth-teapot` | Fixed glossy Phong; seven-second painter/depth cycle on matched motion, overlap artefacts and timing/memory tradeoff | Approved on S3 |
| 03c | `esp32-neon-car` | Runtime OBJ/MTL loading, indexed livery, textured glossy Phong, view-dependent environment mapping on separate windows, neon workshop | Approved on S3 |
| 04 | `esp32-shading-gallery` | Flat, Gouraud, Phong, unlit and wireframe comparisons; per-face materials; coloured ambient and depth brightness | Planned |
| 05 | `esp32-postfx-crt` | Seven-second CRT off/on comparison, bright arcade test card, matched rotating cube, physical-row scanlines without an extra buffer | Approved on S3 |
| 06 | `esp32-postfx-cel` | Seven-second smooth/cel comparison on the glossy red teapot, four diffuse bands, no extra effect buffer | Approved on S3 |
| 07 | `esp32-particles` | Four stages: additive sparks, water spray, 200-slot cap, distance culling; LIVE/DRAWN counters and inclusive render timing | Approved on S3 |
| 08 | `esp32-sprites-blending` | After Hours: mirrored-mesh glossy floor, translucent layering, additive lamp pools/cones, projected quarter-image sprite halos | Approved on S3 |
| 08b | `esp32-sprite-controls` | Air Mail: aircraft flips, shared/per-sprite alpha echoes, animated letterboxing and full-screen fades | Approved on S3 |
| 08c | `esp32-object-blending` | Remaining 3D object blend equations: subtract, scale, average and XOR | Planned |
| 08d | `esp32-lod-billboards` | Woodland: full/simple pine mesh LOD, matched billboard stand-in, distance fade and full-mesh reference pass | Approved on S3 |
| 08e | `esp32-mesh-instancing` | REPEAT: independent mesh copies versus immutable shared instances, measured memory and CPU tradeoff | Hardware checked; awaiting review |
| Film | `esp32-neon-film` | ESP 88: twelve-scene original neon city cinematic, scene asset lifetimes, autonomous coupe, pursuit, wheel-only flight and waterfront credits | Approved |
| Exhibition | `esp32-matter` | MATTER: three-minute procedural art exhibition, kinetic sculpture, analytic surface deformation, chrome, folded paper, phyllotaxis and ceramic tunnel | Approved |
| 09 | `esp32-geometry-depth` | Primitives, billboards, near/far clipping, culling, depth bias, optional Z buffering, picking and distance/LOD fades | Planned |

Every completed project will document its configuration, assets, measured S3
cadence and compromises. Relevant showcase assets stay with their example;
portable reusable helpers can be shared. Buffered full-screen effects and
desktop checkerboard reconstruction are not zero-memory S3 paths: assess them
separately instead of implying the existing field runtime supports them.

Feedback on the currently flashed example gates the next one. This list tracks
the intended coverage; a planned row is not an implemented demo.
