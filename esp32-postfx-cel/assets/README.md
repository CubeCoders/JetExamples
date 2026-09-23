# Utah teapot source

`fg_teapot_data.h` is vendored from the
[FreeGLUT project](https://github.com/freeglut/freeglut/blob/master/src/fg_teapot_data.h).
It contains Martin Newell's Utah teapot control points and a permissive licence,
retained verbatim in that file. Downloaded 23 September 2026. The generated
header records the exact input's SHA-256 hash.

`../tools/generate_teapot.py` evaluates the teapot's ten input Bezier patches,
including their rotated/reflected copies, and produces the 32-patch solid.
Five subdivisions per patch yield 822 vertices and 1,560 non-degenerate
triangles. Analytic derivatives supply smooth normals, with special handling
for the lid and bottom poles. The mesh is centred vertically and scaled for
Jet's integer world coordinates. No model parsing or tessellation runs on S3.

Regenerate with `python tools/generate_teapot.py` from the example directory.
The script uses only Python's standard library and the vendored data.
