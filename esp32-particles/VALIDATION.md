# Particle Lab validation

Built with ESP-IDF 6.0.1 and flashed to the connected ESP32-S3 on 2026-09-23.
Jet's optional additive sparks and accepted draw counters are committed locally
as **fffbf63**. The firmware was built from those changes before the commit.

## Native correctness

Both Release CTests pass with assertions enabled. Pool tests cover requests above
capacity, occupied-slot preservation, dead-slot reuse, gravity integration,
lifetime expiry, additive sparks versus alpha-over water, both field parities,
distance/projection/fade culling, reset counters and framebuffer guards.

The scene test advances the complete 28-second cycle in 120 Hz steps. It reaches
all 200 slots, checks the live-but-culled state at the far camera position, and
compares exact pixels and inclusive triangle counts in 12 serial/parallel cases.
The six-view montage was visually inspected, including the widened water spray.

The shared runtime now includes the optional render-effects callback in measured
render time and adds its returned triangle count to the scene count. The existing
CRT showcase also builds successfully on S3 while leaving this callback unused.
The DMA scanout, pacing and parallel opaque renderer are unchanged.

## Hardware capture

A 34-second capture includes all four stages and the return to sparks. Across
32 reporting windows, cadence averaged **59.97 fields/s**, with a range
of **59.96-59.98**. No panics, unexpected resets or idle recovery yields
were observed. Representative settled render times were:

| Stage | Render time |
| --- | --- |
| Additive sparks | 2.13-2.14 ms |
| Water spray | 2.10-2.13 ms |
| Pool-limit bursts | 2.88-2.97 ms |
| Distance-cull far endpoint | about 1.12 ms |

Times include opaque scene rendering and serial particle drawing. Particle
physics/emission updates remain outside the render timer. Scanout takes about
16.30-16.33 ms. Both the accepted scene and particle triangles contribute to
TRIS and actual-render TRI/S. LIVE/DRAWN counters separately show pool occupancy
and accepted particle triangles. Reporting windows can span mode switches.

The static particle array is **7,200 bytes** (200 slots). Startup memory reports
**51,383 bytes internal** and **8,377,952 bytes PSRAM** free. There is no depth
buffer or per-particle heap allocation. Counter digits and labels use immutable
flash textures. Firmware is **0x704f0 bytes**, leaving 56% of the 1 MiB application
partition free. The board's existing physical 16 MiB / configured 8 MiB flash
warning remains unchanged.

The example was visually approved on S3 before starting After Hours.
P4 defaults are supplied but were not tested on hardware.
