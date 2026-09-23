# Woodland assets

The pine meshes and billboard are original procedural artwork under the
examples repository MIT licence. The mesh is one closed, connected surface of
revolution: trunk, spreading lower branches and progressively narrower tiers.
Adjacent tiers share rings instead of intersecting volumes. The generator
checks that each welded edge has exactly two incident faces.

The full mesh has 16 radial segments and 224 source triangles. The simple mesh
has 6 segments and 84 triangles. Backface culling reduces the number submitted
for rasterization. Face colours approximate a fixed directional light.

`pine-billboard.png` is an orthographic front-view bake of the full pine.
Black is the transparency key. The 128x160 RGB565 header is vertically flipped
for Jet's billboard UV convention; the PNG remains upright. The image occupies
40 KiB and is shared by the featured tree and all six background trees.
The billboard is 512x640 world units, centred at the same Y=280 as the mesh.

The generator rasterizes captions with a chosen local font. The font itself
is not distributed; use `--font` to select another TrueType font.
