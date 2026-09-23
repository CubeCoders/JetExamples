# Crate artwork

- Artwork: [2D] Wooden Box, by Cpt_Flash, published 2013-08-13.
- Source and verified licence: https://opengameart.org/content/2d-wooden-box
- Original download: https://opengameart.org/sites/default/files/RTS_Crate.png
- User-supplied preview: https://opengameart.org/sites/default/files/styles/medium/public/RTS_Crate_0.png
- Licence: CC0 1.0, https://creativecommons.org/publicdomain/zero/1.0/

The unmodified 512x512 source is retained as `RTS_Crate.png`. The generated
`main/CrateTexture.hpp` contains a 128x128 RGB565 conversion (32,768 bytes), with
box downsampling and the source SHA256 recorded in its header. All cube faces
and four sampling modes borrow the same immutable texture storage.

To regenerate, install Pillow and run `python tools/import_crate.py` from this
example. The firmware build uses the checked-in header and does not require Python
image packages. Screen labels are pre-rasterized text; no font is distributed.
