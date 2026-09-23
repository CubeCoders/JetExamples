#include "Cube.hpp"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <vector>

int main() {
    constexpr int width = Display::RENDER_WIDTH, height = Display::RENDER_HEIGHT;
    constexpr int stride = HALF_WIDTH_BUFFERS ? width / 2 : width;
    const size_t count = stride * height / 2;
    std::vector<uint16_t> buffers[2] = {std::vector<uint16_t>(count + 16, 0x0841),
                                       std::vector<uint16_t>(count + 16, 0x0841)};
    Renderer::Scene scene(buffers[0].data(), nullptr, width, height);
    scene.getRenderer()->interlacedMode = true;
    Cube::init(scene);
    int objects, triangles, vertices;
    scene.getStatistics(objects, triangles, vertices);
    assert(objects == 1 && triangles == 12);
    uint64_t previous = 0;
    unsigned changed = 0;
    for (int frame = 0; frame < 120; ++frame) {
        auto& buffer = buffers[frame % 2];
        scene.setFramebuffer(buffer.data());
        scene.getRenderer()->reflectBuffer = buffers[(frame + 1) % 2].data();
        Cube::update(1.0f / 60.0f);
        scene.render();
        const auto pixels = std::count_if(buffer.begin(), buffer.begin() + count,
                                         [](auto p) { return p != 0x0841; });
        assert(pixels > 1000 && pixels < int(count / 2));
        assert(scene.lastFrameDrawnObjects == 1 && scene.lastFrameRasterizedTriangles > 0);
        assert(std::all_of(buffer.begin() + count, buffer.end(), [](auto p) { return p == 0x0841; }));
        uint64_t hash = 1469598103934665603ULL;
        for (size_t i = 0; i < count; ++i) hash = (hash ^ buffer[i]) * 1099511628211ULL;
        changed += hash != previous;
        previous = hash;
    }
    assert(changed > 90);
    // Scene frame 0 writes odd rows; frame 1 writes even rows. Reconstruct
    // the last pair exactly as scanout does, at full output resolution.
    FILE* preview = std::fopen("cube.ppm", "wb");
    assert(preview);
    std::fprintf(preview, "P6\n%d %d\n255\n", width, height);
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
        const uint16_t pixel = buffers[(y & 1) ? 0 : 1][(y / 2) * stride + (HALF_WIDTH_BUFFERS ? x / 2 : x)];
        const unsigned char rgb[] = {static_cast<unsigned char>(((pixel >> 11) & 31) * 255 / 31),
            static_cast<unsigned char>(((pixel >> 5) & 63) * 255 / 63),
            static_cast<unsigned char>((pixel & 31) * 255 / 31)};
        std::fwrite(rgb, 1, 3, preview);
    }
    std::fclose(preview);
    std::puts("Cube: 120 rotating fields, one object, 12 triangles, guard pixels intact");
}
