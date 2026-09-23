# Woodland assets

The oak tree, simple tree and billboard are original procedural artwork under
the examples repository MIT licence. The mesh generator builds a tapered trunk
and five ellipsoidal foliage clusters, with face colours calculated from a
fixed light direction. The reduced mesh uses fewer longitude/latitude segments.

`oak-billboard.png` is an orthographic front-view bake of the high-detail tree's
triangles, generated offline with painter ordering. Black is the transparency
key. The 128x160 RGB565 header is flipped vertically to match the UV convention
of Jet's billboard primitive; the PNG remains upright for inspection.
The billboard is 512x640 world units, centred at the same Y=280 as the mesh.

The generator rasterizes captions with the chosen local font. The font itself
is not distributed; use `--font` to select another TrueType font.
