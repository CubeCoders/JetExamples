#include "Frame.hpp"
#include <array>
int main() {
    Frame f;
    std::array<size_t, 10> sizes{};
    size_t maxTris = 0;
    for (int pass = 0; pass < 3; ++pass)
        for (int c = 0; c < 10; ++c) {
            f.render(c * 18 + 5);
            if (!pass)
                sizes[c] = Matter::bank.objects.size();
            assert(sizes[c] == Matter::bank.objects.size());
            assert(Matter::chapter == c);
            assert(f.target.getSprites().size() == 2 + Matter::glows.size());
            // Both uses of the closed knot must cull the inward-facing side,
            // leaving the hemisphere whose smooth normals face the camera.
            if (c == 8 || c == 9)
                for (auto &item : Matter::items)
                    if (item.kind == (c == 8 ? 1 : 0))
                        for (auto &tri : item.o->triangles) {
                            auto &a = item.o->vertices[tri.v1];
                            auto &b = item.o->vertices[tri.v2];
                            auto &d = item.o->vertices[tri.v3];
                            auto face =
                                (Matter::vector(b.position) - Matter::vector(a.position))
                                    .cross(Matter::vector(d.position) - Matter::vector(a.position));
                            auto normal = Matter::vector(a.normal) + Matter::vector(b.normal) +
                                          Matter::vector(d.normal);
                            assert(face.x * normal.x + face.y * normal.y + face.z * normal.z > 0);
                        }
            auto original = f.out;
            f.render(c * 18 + 5);
            if (original != f.out) {
                Frame::save("first.ppm", original);
                Frame::save("repeat.ppm", f.out);
                std::fprintf(stderr, "Serial repeat differs at %d\n", c);
                return 3;
            }
            f.render(c * 18 + 5, true);
            if (original != f.out) {
                Frame::save("serial.ppm", original);
                Frame::save("parallel.ppm", f.out);
                int changed = 0;
                for (size_t k = 0; k < original.size(); ++k)
                    changed += original[k] != f.out[k];
                std::fprintf(stderr, "Parallel differs at %d: %d pixels\n", c, changed);
                return 4;
            }
        }
    for (int frame = 0; frame <= 1800; ++frame) {
        float t = frame * .1f;
        f.render(t);
        maxTris = std::max(maxTris, size_t(f.target.lastFrameDrawnTriangles));
        for (auto &o : Matter::bank.objects) {
            assert(o->vertices.size() < 65536);
            for (auto &tri : o->triangles) {
                assert(tri.material);
                assert(tri.v1 < o->vertices.size() && tri.v2 < o->vertices.size() &&
                       tri.v3 < o->vertices.size());
            }
        }
        if (frame % 180 == 0) {
            std::printf("Timeline %.1f\n", t);
            std::fflush(stdout);
        }
    }
    // Dynamic geometry must reproduce the same pose after backwards seeking.
    for (float t : {42.5f, 61.25f, 115.7f}) {
        f.render(t);
        auto expected = f.out;
        f.render(t + 2);
        f.render(t);
        assert(expected == f.out);
    }
    // All moving vertices stay within their conservative local-space bounds.
    for (float t : {38.f, 44.f, 57.f, 62.f, 113.f, 122.f}) {
        Matter::seek(t);
        for (auto &o : Matter::bank.objects) {
            if (o->cachedPositions())
                continue;
            for (auto &v : o->vertices) {
                assert(v.position.x >= o->boundingBoxMin.x && v.position.x <= o->boundingBoxMax.x);
                assert(v.position.y >= o->boundingBoxMin.y && v.position.y <= o->boundingBoxMax.y);
                assert(v.position.z >= o->boundingBoxMin.z && v.position.z <= o->boundingBoxMax.z);
            }
        }
    }
    f.render(180);
    assert(std::all_of(f.out.begin(), f.out.end(), [](uint16_t p) { return p == 0; }));
    Matter::update(.5f);
    assert(!Matter::finished);
    Matter::update(.6f);
    assert(Matter::finished);
    std::printf("180-second timeline, buffer guards, painter bounds, deterministic seeking, "
                "parallel raster and repeated scene release pass; max %zu queued triangles\n",
                maxTris);
}
