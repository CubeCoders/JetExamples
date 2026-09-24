#include "Instancing.hpp"
#include "PerformanceOverlay.hpp"
#include <cassert>
#include <fstream>
#include <thread>
static constexpr int w = 480, h = 320, n = w * h / 4, guard = 64;
static void parallel(Renderer::Scene &scene) {
    std::vector<uint8_t> upper(scene.lastFrameDrawnTriangles), lower(upper.size());
    std::thread worker([&] { scene.rasterizeBand(160, h, lower.data()); });
    scene.rasterizeBand(0, 160, upper.data());
    worker.join();
    scene.lastFrameRasterizedTriangles = 0;
    for (size_t i = 0; i < upper.size(); ++i)
        scene.lastFrameRasterizedTriangles += bool(upper[i] | lower[i]);
}
static void save(const char *path, const std::vector<uint16_t> &pixels, int width = w,
                 int height = h) {
    std::ofstream f(path, std::ios::binary);
    f << "P6\n" << width << " " << height << "\n255\n";
    for (auto p : pixels) {
        char c[] = {char(((p >> 11) & 31) * 255 / 31), char(((p >> 5) & 63) * 255 / 63),
                    char((p & 31) * 255 / 31)};
        f.write(c, 3);
    }
}
int main() {
    std::vector<uint16_t> pixels(n + 2 * guard, 0xA55A);
    Renderer::Scene scene(pixels.data() + guard, nullptr, w, h);
    scene.getRenderer()->interlacedMode = true;
    Instancing::init(scene);
    PerformanceOverlay hud;
    hud.attach(scene, w);
    auto render = [&](float time, bool threaded, bool labels) {
        Instancing::seek(time);
        std::vector<uint16_t> full(w * h);
        for (int parity = 0; parity < 2; ++parity) {
            scene.frameCounter = parity;
            scene.render(threaded ? parallel : nullptr);
            for (int i = 0; i < guard; ++i)
                assert(pixels[i] == 0xA55A && pixels[n + guard + i] == 0xA55A);
            for (int y = 1 - parity; y < h; y += 2)
                for (int x = 0; x < w; ++x)
                    full[y * w + x] = pixels[guard + y / 2 * (w / 2) + x / 2];
        }
        if (labels) {
            auto sprites = scene.getSprites();
            std::stable_sort(sprites.begin(), sprites.end(),
                             [](auto *a, auto *b) { return a->zOrder < b->zOrder; });
            for (int y = 0; y < h; ++y)
                Renderer::compositeSprites(full.data() + y * w, w, y, sprites.data(),
                                           int(sprites.size()));
        }
        return full;
    };
    Instancing::selectMode(0);
    size_t separate = Instancing::meshBytes;
    assert(Instancing::objects[0]->cachedPositions() != Instancing::objects[1]->cachedPositions());
    for (float t : {0.f, .8f, 1.9f, 2.6f, 4.f, 5.3f, 6.2f, 7.8f}) {
        auto a = render(t, false, false);
        int tris = scene.lastFrameRasterizedTriangles;
        auto b = render(t + 8, false, false);
        if (a != b) {
            save("mismatch-copies.ppm", a);
            save("mismatch-instances.ppm", b);
            std::fprintf(stderr, "Mismatch at %.3f seconds\n", t);
        }
        assert(a == b);
        assert(tris == scene.lastFrameRasterizedTriangles);
        assert(b == render(t + 8, true, false));
        assert(Instancing::meshBytes * Instancing::count == separate);
        for (auto &o : Instancing::objects) {
            assert(o->vertices.empty() && o->triangles.empty());
            assert(o->instances.size() == 1);
            assert(o->instances[0].mesh == Instancing::prototype);
        }
    }
    std::weak_ptr<const Renderer::Object> released = Instancing::prototype;
    for (int i = 0; i < 64; ++i) {
        Instancing::selectMode(i % 2);
        Instancing::pose(i * .125f);
        scene.render();
    }
    assert(released.expired());
    auto a = render(2.6f, false, true), b = render(10.6f, false, true);
    save("copies.ppm", a);
    save("instances.ppm", b);
    std::vector<uint16_t> side(w * h * 2);
    for (int y = 0; y < h; ++y) {
        std::copy_n(a.data() + y * w, w, side.data() + y * w * 2);
        std::copy_n(b.data() + y * w, w, side.data() + y * w * 2 + w);
    }
    save("instancing.ppm", side, w * 2, h);
    std::printf("Instancing: %u -> %u mesh bytes; identical pixels at eight poses, serial/parallel "
                "agreement, guards and 64 ownership switches passed\n",
                unsigned(separate), unsigned(Instancing::meshBytes));
    Instancing::clearMeshes();
}
