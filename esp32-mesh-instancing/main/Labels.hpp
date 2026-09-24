#pragma once
#include "Scene.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <vector>
namespace Labels {
using namespace Renderer;
inline const uint8_t *glyph(char c) {
    static constexpr uint8_t letters[][5] = {
        {126, 17, 17, 17, 126}, {127, 73, 73, 73, 54}, {62, 65, 65, 65, 34},  {127, 65, 65, 34, 28},
        {127, 73, 73, 73, 65},  {127, 9, 9, 9, 1},     {62, 65, 73, 73, 122}, {127, 8, 8, 8, 127},
        {0, 65, 127, 65, 0},    {32, 64, 65, 63, 1},   {127, 8, 20, 34, 65},  {127, 64, 64, 64, 64},
        {127, 2, 12, 2, 127},   {127, 4, 8, 16, 127},  {62, 65, 65, 65, 62},  {127, 9, 9, 9, 6},
        {62, 65, 81, 33, 94},   {127, 9, 25, 41, 70},  {38, 73, 73, 73, 50},  {1, 1, 127, 1, 1},
        {63, 64, 64, 64, 63},   {31, 32, 64, 32, 31},  {63, 64, 56, 64, 63},  {99, 20, 8, 20, 99},
        {7, 8, 112, 8, 7},      {97, 81, 73, 69, 67}};
    static constexpr uint8_t digits[][5] = {
        {62, 81, 73, 69, 62},  {0, 66, 127, 64, 0},  {66, 97, 81, 73, 70}, {33, 65, 69, 75, 49},
        {24, 20, 18, 127, 16}, {39, 69, 69, 69, 57}, {60, 74, 73, 73, 48}, {1, 113, 9, 5, 3},
        {54, 73, 73, 73, 54},  {6, 73, 73, 41, 30}};
    static constexpr uint8_t blank[5] = {}, dash[] = {8, 8, 8, 8, 8}, dot[] = {0, 96, 96, 0, 0};
    if (c >= 'A' && c <= 'Z')
        return letters[c - 'A'];
    if (c >= '0' && c <= '9')
        return digits[c - '0'];
    return c == '-' ? dash : c == '.' ? dot : blank;
}
struct Label {
    std::vector<uint16_t> pixels;
    Texture texture{1, 1, nullptr, true, 0};
    Material material{0xffff, &texture};
    void init(int w, int h) {
        pixels.assign(w * h, 0);
        texture.width = w;
        texture.height = h;
        texture.data = pixels.data();
    }
    void text(const char *s, int x, int y, int scale = 1, uint16_t colour = 0xffff) {
        for (; *s; ++s, x += 6 * scale)
            for (int xx = 0; xx < 5; ++xx)
                for (int yy = 0; yy < 7; ++yy)
                    if (glyph(*s)[xx] & (1 << yy))
                        for (int dy = 0; dy < scale; ++dy)
                            for (int dx = 0; dx < scale; ++dx) {
                                int px = x + xx * scale + dx, py = y + yy * scale + dy;
                                if (px >= 0 && px < texture.width && py >= 0 && py < texture.height)
                                    pixels[py * texture.width + px] = colour;
                            }
    }
};
inline Label title, footer;
inline std::array<Label, 2> modes;
inline Sprite2D titleSprite, modeSprite, footerSprite, top, bottom, progress;
inline Material panel{0x10e4}, accent{0xDDAF};
inline int slot = 0;
inline void init(Scene &scene) {
    title.init(300, 34);
    title.text("REPEAT", 0, 0, 3, 0xf6f8);
    title.text("SHARED MESH INSTANCING", 0, 26);
    footer.init(444, 8);
    footer.text("SAME PICTURE - LESS MEMORY - MEASURE THE COST", 0, 0, 1, 0xBDF7);
    for (auto &mode : modes)
        mode.init(360, 33);
    top = makeSolidRect(0, 0, 480, 64, &panel);
    bottom = makeSolidRect(0, 262, 480, 58, &panel);
    progress = makeSolidRect(18, 256, 0, 3, &accent);
    for (auto *s : {&top, &bottom, &progress}) {
        s->zOrder = 20;
        scene.addSprite(s);
    }
    titleSprite.x = 18;
    titleSprite.y = 12;
    titleSprite.material = &title.material;
    modeSprite.x = 18;
    modeSprite.y = 270;
    footerSprite.x = 18;
    footerSprite.y = 308;
    footerSprite.material = &footer.material;
    for (auto *s : {&titleSprite, &modeSprite, &footerSprite}) {
        s->zOrder = 30;
        scene.addSprite(s);
    }
}
inline void select(int mode, int count, size_t bytes) {
    // Scanout still borrows the previous label's pixels. Write the other buffer,
    // then swap materials; never resize/free either pixel buffer while running.
    slot ^= 1;
    auto &label = modes[slot];
    std::fill(label.pixels.begin(), label.pixels.end(), 0);
    char text[80];
    std::snprintf(text, sizeof(text), mode ? "%d INSTANCES - 1 MESH" : "%d COPIES - %d MESHES",
                  count, count);
    label.text(text, 0, 0, 2, mode ? 0xAEB8 : 0xF5AF);
    std::snprintf(text, sizeof(text), "MESH DATA %u.%u KIB - SAME %d ROTORS",
                  unsigned(bytes / 1024), unsigned(bytes * 10 / 1024 % 10), count);
    label.text(text, 0, 23);
    modeSprite.material = &label.material;
}
} // namespace Labels
