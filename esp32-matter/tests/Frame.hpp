#pragma once
#include "Exhibition.hpp"
#include <cassert>
#include <fstream>
#include <thread>
struct Frame {
    static constexpr int w = 480, h = 320, n = w * h / 4, guard = 128;
    std::vector<uint16_t> a, b, out;
    Renderer::Scene target;
    Frame()
        : a(n + guard * 2, 0xA55A), b(n + guard * 2, 0xA55A), out(w * h),
          target(a.data() + guard, nullptr, w, h) {
        target.getRenderer()->interlacedMode = true;
        Matter::init(target);
    }
    ~Frame() {
        Matter::clearGlows();
        Matter::bank.clear();
    }
    static void parallel(Renderer::Scene &s) {
        std::vector<uint8_t> a(s.lastFrameDrawnTriangles), b(a.size());
        std::thread worker([&] { s.rasterizeBand(h / 2, h, b.data()); });
        s.rasterizeBand(0, h / 2, a.data());
        worker.join();
        s.lastFrameRasterizedTriangles = 0;
        for (size_t i = 0; i < a.size(); ++i)
            s.lastFrameRasterizedTriangles += bool(a[i] | b[i]);
    }
    const std::vector<uint16_t> &render(float time, bool threaded = false) {
        Matter::seek(time);
        for (int parity = 0; parity < 2; ++parity) {
            auto &write = parity ? b : a;
            auto &read = parity ? a : b;
            target.frameCounter = parity;
            target.setFramebuffer(write.data() + guard);
            target.getRenderer()->reflectBuffer = read.data() + guard;
            target.render(threaded ? parallel : nullptr);
            Matter::effects(target);
            for (int i = 0; i < guard; ++i)
                assert(write[i] == 0xA55A && write[n + guard + i] == 0xA55A);
        }
        for (int y = 0; y < h; ++y) {
            auto &src = y % 2 ? a : b;
            for (int x = 0; x < w; ++x)
                out[y * w + x] = src[guard + y / 2 * (w / 2) + x / 2];
        }
        auto sprites = target.getSprites();
        std::stable_sort(sprites.begin(), sprites.end(),
                         [](auto *a, auto *b) { return a->zOrder < b->zOrder; });
        for (int y = 0; y < h; ++y)
            Renderer::compositeSprites(out.data() + y * w, w, y, sprites.data(),
                                       int(sprites.size()));
        return out;
    }
    static void save(const char *path, const std::vector<uint16_t> &pixels, int ww = w,
                     int hh = h) {
        std::ofstream file(path, std::ios::binary);
        file << "P6\n" << ww << " " << hh << "\n255\n";
        for (auto p : pixels) {
            char c[] = {char(((p >> 11) & 31) * 255 / 31), char(((p >> 5) & 63) * 255 / 63),
                        char((p & 31) * 255 / 31)};
            file.write(c, 3);
        }
    }
};
