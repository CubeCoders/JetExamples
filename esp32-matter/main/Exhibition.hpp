#pragma once
#include "Surfaces.hpp"
#include "Typography.hpp"
#include <cstdio>
#if defined(ESP_PLATFORM)
#include <esp_system.h>
#include <esp_heap_caps.h>
#endif
namespace Matter {
inline constexpr float duration = 180;
inline constexpr int chapterCount = 10;
inline const char *names[] = {"PRESSURE",        "COUNTERWEIGHT", "PAPER WEATHER", "QUICKSILVER",
                              "COLOUR IN SPACE", "BOTANICA",      "INTERFERENCE",  "RELIQUARY",
                              "THE GYRE",        "MATTER"};
inline int chapter = -1;
inline float time = 0, local = 0;
inline bool finished = false;
struct Item {
    Object *o;
    Vector3 base;
    int kind;
    float phase;
};
inline std::vector<Item> items;
inline std::vector<Object *> reflective;
inline Object *landscape = nullptr;
inline Material fadeMaterial(0, uint8_t(255));
inline Sprite2D fade;
inline Material textMaterial;
inline Sprite2D text;
inline Texture textTexture(1, 1, nullptr, true, 0);
inline std::vector<std::vector<uint16_t>> typePixels;
inline std::vector<uint16_t> darkPaperType, darkColourType;
inline void typography() {
    if (!typePixels.empty())
        return;
    for (int i = 0; i < Type::count; ++i) {
        auto &pixels = typePixels.emplace_back(Type::widths[i] * Type::heights[i], 0);
        for (size_t j = 0; j < pixels.size(); ++j)
            if (Type::data[i][j / 8] & (0x80 >> (j % 8)))
                pixels[j] = rgb(0xEFE6D3);
    }
    darkPaperType = typePixels[3];
    darkColourType = typePixels[5];
    for (auto *pixels : {&darkPaperType, &darkColourType})
        for (auto &pixel : *pixels)
            if (pixel)
                pixel = rgb(0x142820);
    textMaterial.diffuseMap = &textTexture;
    textMaterial.shadingMode = ShadingMode::UNLIT;
    text.material = &textMaterial;
    text.zOrder = 800000;
    text.scale = renderScale;
    scene->addSprite(&text);
}
inline void lettering(int which, int x, int y, int alpha) {
    textTexture.width = Type::widths[which];
    textTexture.height = Type::heights[which];
    textTexture.data = which == 3   ? darkPaperType.data()
                       : which == 5 ? darkColourType.data()
                                    : typePixels[which].data();
    text.x = x * renderScale;
    text.y = y * renderScale;
    setSolidRectAlpha(text, uint8_t(std::clamp(alpha, 0, 255)));
}
inline void item(Object *o, int kind = 0, float phase = 0) {
    items.push_back({o, o->position, kind, phase});
}
inline Material *glossy(unsigned c) {
    auto *m = bank.paint(c, ShadingMode::PHONG);
    m->specular = 175;
    m->specularExponent = 28;
    m->diffuse = 200;
    return m;
}
inline void shadow(float radius, int y = -417) {
    auto *m = bank.paint(0x141915, ShadingMode::UNLIT, 55);
    auto *o = bank.mesh();
    o->cullingMode = CullingMode::NO_CULLING;
    for (int i = 0; i < 24; ++i) {
        float a = tau * i / 24, b = tau * (i + 1) / 24;
        tri(o, {0, float(y), 0}, {radius * std::cos(a), float(y), radius * .65f * std::sin(a)},
            {radius * std::cos(b), float(y), radius * .65f * std::sin(b)}, m);
    }
    bank.finish(o);
}
inline void pressure() {
    gradient(0x111B1D, 0x657169);
    ambient.color = {91, 99, 97};
    light.updateDirection({50, 40, 0});
    auto *steel = glossy(0xC9AA72);
    auto *edge = glossy(0xC6D1CB);
    steel->specular = edge->specular = 210;
    steel->specularExponent = edge->specularExponent = 42;
    auto *rim = bank.paint(0xB6C4BC, ShadingMode::GOURAUD);
    auto *red = bank.paint(0xC75227, ShadingMode::GOURAUD);
    floor(bank.paint(0x303C39), bank.paint(0x35433E), 2800, 8, -470);
    shadow(685, -467);
    for (int i = 0; i < 3; ++i) {
        auto *o = gear(i == 0 ? 210 : 130, i == 0 ? 10 : 8, steel, edge);
        o->position = {i == 0 ? 0 : i == 1 ? 270 : -270, i == 0 ? 235 : -5, i == 0 ? 0 : 20};
        item(o, i == 0 ? 1 : -1, i * 31.f);
    }
    for (int i = 0; i < 8; ++i) {
        float a = tau * i / 8;
        auto *o = cylinder(32, 100, 8, red);
        aim(o, {std::cos(a) * 620, -430, std::sin(a) * 620},
            {std::cos(a) * 620, 0, std::sin(a) * 620});
        item(o, 2, float(i));
    }
    for (int i = 0; i < 3; ++i) {
        // Disjoint concentric shells enclose the gears at every orientation.
        auto *o = ring(485 + i * 34, 12, 32, 4, i % 2 ? red : rim, {0, 180, 0});
        o->rotation = {65, i * 25, 0};
        item(o, 3, float(i));
    }
    cylinder(680, 36, 40, bank.paint(0x53645B), {0, -450, 0});
    addGlow({-300, 280, 200});
    addGlow({310, -150, -100});
}
inline void counterweight() {
    gradient(0x233349, 0xB3C5C8);
    ambient.color = {119, 126, 143};
    auto *chalk = bank.paint(0xCDC6B3);
    auto *dark = bank.paint(0x48586B);
    auto *ochre = bank.paint(0xD59C34);
    auto *clay = bank.paint(0xB45436);
    floor(bank.paint(0x606F76), bank.paint(0x64777C), 8000, 14, -540);
    // A narrowing nave of independently rising slabs, with generous camera room.
    for (int i = 0; i < 18; ++i)
        for (int side : {-1, 1}) {
            int z = i * 410;
            auto *o = box({side * (700 + (i % 3) * 70), -90, z}, {180, 780 + (i % 4) * 100, 250},
                          i % 4 ? chalk : ochre);
            item(o, 0, float(i) * .49f + side);
            auto *lintel = box({side * 490, 460, z}, {340, 90, 250}, dark);
            item(lintel, 1, float(i) * .49f + side);
        }
    for (int i = 0; i < 10; ++i) {
        // The complete rotating cube stays below the lintel's lowest position
        // and inside the columns, leaving the central camera aisle clear.
        auto *o = box({i % 2 ? 400 : -400, -100 + i % 2 * 30, 700 + i * 650}, {200, 200, 200},
                      i % 2 ? clay : ochre);
        item(o, 2, float(i));
    }
}
inline void paperWeather() {
    gradient(0x849FA0, 0xE0DAC6);
    ambient.color = {145, 135, 110};
    auto *cream = bank.paint(0xECE0BA, ShadingMode::FLAT);
    auto *coral = bank.paint(0xCF654B, ShadingMode::FLAT);
    auto *teal = bank.paint(0x487C79, ShadingMode::FLAT);
    landscape = bank.mesh();
    landscape->cullingMode = CullingMode::NO_CULLING;
    for (int z = 0; z < 15; ++z)
        for (int x = 0; x < 24; ++x) {
            float xx = (x - 12) * 100, zz = (z - 7) * 100;
            auto *m = (x + z) % 5 == 0 ? coral : cream;
            tri(landscape, {xx, 0, zz + 100}, {xx + 100, 0, zz + 100}, {xx + 100, 0, zz}, m);
            tri(landscape, {xx + 100, 0, zz}, {xx, 0, zz}, {xx, 0, zz + 100}, m);
        }
    bank.finish(landscape);
    landscape->invalidatePositions();
    landscape->boundingBoxMin = {-1400, -500, -1000};
    landscape->boundingBoxMax = {1400, 500, 1000};
    for (int i = 0; i < 26; ++i) {
        auto *o = bank.mesh();
        tri(o, {-72, 0, 0}, {0, 14, 30}, {0, 0, -52}, i % 3 ? cream : teal);
        tri(o, {0, 0, -52}, {0, 14, 30}, {72, 0, 0}, i % 4 ? coral : cream);
        o->cullingMode = CullingMode::NO_CULLING;
        bank.finish(o);
        o->invalidatePositions();
        o->boundingBoxMin = {-100, -100, -100};
        o->boundingBoxMax = {100, 100, 100};
        item(o, 0, float(i));
    }
}
inline void quicksilver() {
    gradient(0x182724, 0x879B8B);
    ambient.color = {96, 117, 111};
    floor(bank.paint(0x50665C), bank.paint(0x63766A), 2200, 6);
    shadow(500);
    auto *chrome = bank.texture(&room);
    blob.build(chrome, 20);
    blob.object->position.y = 160;
    reflective.push_back(blob.object);
    auto *porcelain = glossy(0xE5D6AB);
    for (int i = 0; i < 2; ++i) {
        auto *o = ring(415 + i * 75, 16, 32, 5, porcelain, {0, 160, 0});
        o->rotation = {i * 56 + 25, i * 43, 0};
        item(o, 0, float(i));
    }
    for (int i = 0; i < 3; ++i) {
        auto *o = sphere(42 + i % 2 * 5, 10, chrome);
        item(o, 1, float(i));
        reflective.push_back(o);
    }
    for (int i = 0; i < 7; ++i)
        box({(i - 3) * 205, 110, -720}, {110, 1060 - (i % 2) * 200, 180},
            bank.paint(i % 2 ? 0x65796B : 0xB2B49A));
    cylinder(390, 35, 36, bank.paint(0xC5BA97), {0, -398, 0});
}
inline void colourSpace() {
    gradient(0xDCB7A1, 0xF1D6B3);
    ambient.color = {142, 129, 105};
    auto *blue = glossy(0x174994);
    auto *red = bank.paint(0xDA4934, ShadingMode::GOURAUD);
    auto *yellow = bank.paint(0xEBC339);
    auto *black = bank.paint(0x282C36);
    auto *ivory = bank.paint(0xF4EACD);
    floor(ivory, bank.paint(0xC79980), 2800, 12);
    shadow(500);
    for (int i = 0; i < 3; ++i) {
        auto *o = ring(170, 45, 32, 6,
                       i == 0   ? blue
                       : i == 1 ? yellow
                                : red,
                       {-420 + i * 420, i == 1 ? 240 : -30, -100});
        o->rotation = {90, 0, i * 40};
        item(o, 0, float(i));
    }
    for (int i = 0; i < 15; ++i) {
        auto *o = box({(i - 7) * 77, -250, 310}, {51, 290, 65},
                      i % 3 == 0   ? blue
                      : i % 3 == 1 ? red
                                   : yellow);
        item(o, 1, float(i));
    }
    for (int i = 0; i < 5; ++i) {
        auto *o = sphere(65, 12, ivory);
        item(o, 2, float(i));
    }
    box({-650, -200, 200}, {60, 430, 90}, black);
    box({650, -200, 200}, {60, 430, 90}, black);
    for (int i = 0; i < 4; ++i) {
        auto *o = box({0, 0, 0}, {540, 20, 30}, black);
        item(o, 3, float(i));
    }
}
inline void botanica() {
    gradient(0x243831, 0xABB28A);
    ambient.color = {110, 123, 90};
    auto *green = glossy(0x577A4C);
    auto *copper = bank.paint(0xB77C3C);
    auto *ivory = bank.paint(0xDACD9C);
    floor(bank.paint(0x687357), bank.paint(0x727B5D), 2600, 8, -490);
    shadow(460, -487);
    for (int i = 0; i < 72; ++i) {
        auto *o = bank.mesh();
        float size = 53 + (i % 5) * 5;
        tri(o, {0, -size, 0}, {-size * .46f, 0, 0}, {0, size * 1.2f, 14},
            i % 5 == 0 ? copper : green);
        tri(o, {0, -size, 0}, {0, size * 1.2f, 14}, {size * .46f, 0, 0},
            i % 3 == 0 ? ivory : green);
        tri(o, {size * .46f, 0, 0}, {0, size * 1.2f, 14}, {-size * .46f, 0, 0}, green);
        o->cullingMode = CullingMode::NO_CULLING;
        bank.finish(o);
        item(o, 0, float(i));
    }
    auto *seed = sphere(105, 18, glossy(0xD2A348));
    item(seed, 1);
    cylinder(190, 48, 28, bank.paint(0xB0A184), {0, -450, 0});
    for (int i = 0; i < 16; ++i)
        addGlow({0, 0, 0});
}
inline void interference() {
    gradient(0x22252B, 0x807A71);
    ambient.color = {150, 145, 126};
    auto *ivory = bank.paint(0xE3DFCA, ShadingMode::GOURAUD);
    auto *ink = bank.paint(0x161E25, ShadingMode::GOURAUD);
    auto *red = bank.paint(0xD4452F);
    for (int i = 0; i < 3; ++i)
        ribbon(72, 3000, 65, float(i) * tau / 3, i == 1 ? red : ivory, ink);
    for (int i = 0; i < 9; ++i) {
        auto *o = sphere(55, 10, i % 3 ? ivory : red);
        item(o, 0, float(i));
    }
}
inline void reliquary() {
    gradient(0x263747, 0xA1B7B6);
    ambient.color = {95, 118, 130};
    auto *silver = bank.texture(&room);
    auto *blue = bank.paint(0x7AABC0, ShadingMode::FLAT);
    auto *gold = bank.paint(0xBE953E, ShadingMode::FLAT);
    floor(bank.paint(0x354C57), bank.paint(0x45616B), 2400, 10, -480);
    for (int i = 0; i < 40; ++i) {
        auto *o = crystal(45 + (i % 4) * 12, 135 + randf(i * 7) * 160, 5, i % 4 ? blue : gold,
                          i % 4 ? bank.paint(colour(0x283A52, 0xD3E6D4, randf(i * 17))) : silver);
        item(o, 0, float(i));
    }
    auto *core = crystal(205, 310, 8, silver, gold);
    item(core, 1);
    reflective.push_back(core);
    for (int i = 0; i < 3; ++i) {
        auto *o = ring(360 + i * 70, 7, 40, 4, gold);
        item(o, 2, float(i));
    }
    addGlow({-290, 100, 130});
    addGlow({270, -120, -180});
}
inline void gyre() {
    gradient(0x2B333A, 0x948572);
    // The tunnel is unlit. A low fill and a broad side key model the knot
    // independently of the graphic ceramic backdrop.
    ambient.color = {48, 43, 38};
    light.updateDirection({235, 32, 0});
    light.intensity = 255;
    auto *clay = bank.paint(0xB56848, ShadingMode::UNLIT);
    auto *blue = bank.paint(0x507F8A, ShadingMode::UNLIT);
    auto *cream = bank.paint(0xCDBB95, ShadingMode::UNLIT);
    auto *dark = bank.paint(0x26363B, ShadingMode::UNLIT);
    for (int i = 0; i < 12; ++i) {
        auto *o = bank.mesh();
        o->cullingMode = CullingMode::NO_CULLING;
        auto *m = i % 3 == 0 ? clay : i % 3 == 1 ? blue : cream;
        auto point = [](float a, float radius, float z) {
            float r = radius * (1 + .1f * std::cos(a * 4));
            return V{std::cos(a) * r, std::sin(a) * r, z};
        };
        for (int j = 0; j < 16; ++j) {
            float a = tau * j / 16, b = tau * (j + 1) / 16;
            quad(o, point(a, 500, 0), point(b, 500, 0), point(b, 340, 0), point(a, 340, 0), m);
            quad(o, point(a, 340, 0), point(b, 340, 0), point(b, 322, 40), point(a, 322, 40), dark);
        }
        bank.finish(o);
        o->position.z = i * 220;
        item(o, 0, float(i));
    }
    auto *gold = glossy(0xF4D28B);
    gold->diffuse = 225;
    gold->specular = 235;
    gold->specularExponent = 20;
    auto *core = knot(125, 32, 40, 6, gold);
    item(core, 1);
}
inline void assembly() {
    gradient(0x25342F, 0x8B9781);
    ambient.color = {100, 107, 93};
    light.updateDirection({60, 35, 0});
    light.intensity = 235;
    auto *porcelain = glossy(0xE6DBC1);
    auto *orange = bank.paint(0xC26635);
    auto *blue = bank.paint(0x456A85);
    auto *chrome = bank.texture(&room);
    floor(bank.paint(0x546355), bank.paint(0x63715F), 2600, 8, -460);
    shadow(500, -457);
    auto *centre = knot(155, 45, 48, 6, porcelain);
    item(centre, 0);
    for (int i = 0; i < 12; ++i) {
        Object *o;
        if (i % 4 == 0)
            o = crystal(47, 95, 5, chrome, orange);
        else if (i % 4 == 1)
            o = ring(58, 18, 12, 4, blue);
        else if (i % 4 == 2)
            o = box({0, 0, 0}, {100, 100, 100}, orange);
        else
            o = sphere(48, 10, porcelain);
        item(o, 1, float(i));
        if (i % 4 == 0)
            reflective.push_back(o);
    }
    for (int i = 0; i < 3; ++i) {
        auto *o = ring(340 + i * 45, 11, 32, 4, porcelain);
        item(o, 2, float(i));
    }
}
inline void load(int next) {
    clearGlows();
    items.clear();
    reflective.clear();
    ribbons.clear();
    blob.clear();
    landscape = nullptr;
    bank.clear();
    chapter = next;
#if defined(ESP_PLATFORM) && defined(CONFIG_SPIRAM)
    heap_caps_malloc_extmem_enable(128);
#endif
    camera.setFOV(62.f, width);
    camera.nearPlane = 40;
    camera.farPlane = 12000;
    scene->lodScale = 0;
    light.updateDirection({225, 40, 0});
    light.color = {255, 240, 214};
    light.intensity = 225;
    switch (chapter) {
    case 0:
        pressure();
        break;
    case 1:
        counterweight();
        break;
    case 2:
        paperWeather();
        break;
    case 3:
        quicksilver();
        break;
    case 4:
        colourSpace();
        break;
    case 5:
        botanica();
        break;
    case 6:
        interference();
        break;
    case 7:
        reliquary();
        break;
    case 8:
        gyre();
        break;
    default:
        assembly();
        break;
    }
#if defined(ESP_PLATFORM) && defined(CONFIG_SPIRAM)
    heap_caps_malloc_extmem_enable(CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL);
#endif
    int tris = 0;
    for (auto &o : bank.objects)
        tris += int(o->triangles.size());
    std::fprintf(stderr, "MATTER %02d %s / %zu objects / %d triangles\n", chapter + 1,
                 names[chapter], bank.objects.size(), tris);
#if defined(ESP_PLATFORM)
    std::fprintf(stderr, "Matter heap: internal %u, PSRAM %u, largest %u\n",
                 unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)),
                 unsigned(heap_caps_get_free_size(MALLOC_CAP_SPIRAM)),
                 unsigned(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)));
#endif
}
// Jet mesh Euler angles are whole degrees. Exposed spins run at 34–53
// degrees/second; small slow tilts stay fixed so they do not shimmer. Camera
// rotations remain floating point and can move gently.
inline void pose(float t) {
    float p = t / 18;
    if (chapter == 0) {
        orbit(.12f + t * .065f, 1460 - 130 * std::sin(t * .18f), 250 + 90 * std::sin(t * .23f),
              {0, 150, 0});
        for (auto &i : items) {
            if (i.kind == 1 || i.kind == -1)
                i.o->rotation.z = int(t * 48 * i.kind + i.phase);
            else if (i.kind == 2) {
                i.o->scale.y = int(2800 + 1700 * std::sin(t * 1.8f + i.phase));
                i.o->position.y = -430 + i.o->scale.y * 50 / 1024;
            } else {
                i.o->rotation.x = int(65 + i.phase * 8);
                i.o->rotation.z = int(t * 42 * (int(i.phase) % 2 ? -1 : 1));
            }
        }
    }
    if (chapter == 1) {
        float travel = 900 + t * 260;
        camera.setPosition({int(140 * std::sin(t * .2f)), 50 + int(150 * p), int(travel)});
        camera.lookAt({int(-80 * std::sin(t * .3f)), 180, int(travel + 1400)});
        for (auto &i : items) {
            if (i.kind < 2)
                i.o->position.y = i.base.y + int(160 * std::sin(t * 1.2f + i.phase));
            else {
                i.o->rotation = {int(t * 42 + i.phase * 17), int(t * 53 + i.phase * 29),
                                 int(t * 37)};
                i.o->position.y = i.base.y + int(60 * std::sin(t * 1.5f + i.phase));
            }
        }
    }
    if (chapter == 2) {
        orbit(-.9f + t * .06f, 1220, 590 - 150 * p, {0, -90, 0});
        // Shared coordinates need 41 wave samples, not one per triangle corner.
        float columns[25], rows[16];
        for (int x = 0; x < 25; ++x)
            columns[x] = 95 * std::sin((x * 100 - 1200) * .006f + t * .85f) + (x % 2 ? 30 : -30);
        for (int z = 0; z < 16; ++z)
            rows[z] = 50 * std::cos((z * 100 - 700) * .01f - t * .65f);
        for (auto &v : landscape->vertices) {
            v.position.y =
                int(-230 + columns[(v.position.x + 1200) / 100] + rows[(v.position.z + 700) / 100]);
        }
        flatNormals(landscape);
        for (auto &i : items) {
            float a = t * .85f + i.phase * 2.39996f, rad = 180 + i.phase * 20;
            i.o->position = {int(std::sin(a) * rad), int(40 + 140 * std::sin(t * .8f + i.phase)),
                             int(std::cos(a) * rad)};
            i.o->rotation = {12, int(-a * 180 / pi), int(20 * std::sin(t * 2.6f + i.phase))};
            float flap = 40 * std::sin(t * 3.7f + i.phase);
            i.o->vertices[0].position.y = int(flap);
            i.o->vertices[5].position.y = int(flap);
            flatNormals(i.o);
        }
    }
    if (chapter == 3) {
        orbit(-.55f + t * .075f, 1370 + 80 * std::sin(t * .32f), 185 + 100 * std::sin(t * .27f),
              {0, 100, 0});
        blob.pose(t);
        blob.object->rotation = {int(t * 38), int(t * 47), 0};
        for (auto &i : items) {
            if (i.kind == 0) {
                i.o->rotation = {int(25 + i.phase * 56 + t * 41), int(i.phase * 43 - t * 47),
                                 int(t * 37)};
            } else {
                float a = t * .75f + i.phase * tau / 3;
                // Beads stay between the deformed core and the inner hoop.
                i.o->position = {int(330 * std::cos(a)), int(160 + 65 * std::sin(a * 1.7f)),
                                 int(330 * std::sin(a))};
            }
        }
    }
    if (chapter == 4) {
        orbit(-.5f + t * .075f, 1470, 430 - 150 * p, {0, 25, 0});
        for (auto &i : items) {
            if (i.kind == 0) {
                i.o->rotation = {90, int(t * 47 + i.phase * 35), int(i.phase * 35)};
                i.o->position.y = i.base.y + int(35 * std::sin(t * 1.3f + i.phase));
            } else if (i.kind == 1) {
                i.o->scale = {1024, int(1024 + 600 * std::sin(t * 1.6f + i.phase * .6f)), 1024};
                i.o->transformScale = true;
                i.o->position.y = -400 + i.o->scale.y * 145 / 1024;
            } else if (i.kind == 2) {
                float a = t * .75f + i.phase * tau / 5;
                // Independent pendulums above the foreground slats, ahead of
                // the rings' full rotation envelope.
                i.o->position = {int(-400 + 200 * i.phase), int(240 + 50 * std::sin(a)),
                                 int(300 + 25 * std::cos(a))};
            } else {
                // The whole bar sweep is behind the rings. Separate heights
                // keep the four bars from intersecting one another as well.
                i.o->position = {0, int(250 + i.phase * 50), -670};
                i.o->rotation = {0, int(t * 46 + i.phase * 40), 0};
            }
        }
    }
    if (chapter == 5) {
        orbit(-.4f + t * .11f, 1030 + 100 * std::sin(t * .3f), 110 + 120 * std::sin(t * .23f),
              {0, -50, 0});
        for (auto &i : items) {
            if (i.kind == 0) {
                float u = i.phase / 72, a = i.phase * 2.39996f + t * .8f,
                      r = 55 + 260 * std::sin(pi * u) + 45 * std::sin(t * .7f + i.phase * .2f);
                i.o->position = {int(r * std::cos(a)),
                                 int(-390 + 700 * u + 30 * std::sin(t + i.phase * .25f)),
                                 int(r * std::sin(a))};
                i.o->rotation = {int(25 + 12 * std::sin(t * 3.2f + i.phase * .2f)),
                                 int(-a * 180 / pi), int(20 * std::sin(i.phase * 2.39996f))};
            } else {
                i.o->position.y = int(-20 + 35 * std::sin(t));
                i.o->rotation = {int(t * 38), int(t * 51), 0};
            }
        }
    }
    if (chapter == 6) {
        camera.setPosition(
            {int(85 * std::sin(t * .29f)), int(65 * std::cos(t * .24f)), -2420 + int(200 * p)});
        camera.lookAt({0, 0, 400});
        poseRibbons(t);
        for (auto &i : items) {
            float a = t * .65f + i.phase;
            i.o->position = {int(110 * std::sin(a)), int(100 * std::cos(a)),
                             int(-1250 + i.phase * 300)};
        }
    }
    if (chapter == 7) {
        orbit(.1f + t * .11f, 1100, 420 + 100 * std::sin(t * .27f), {0, -60, 0});
        for (auto &i : items) {
            if (i.kind == 0) {
                float a = i.phase * 2.39996f, r = 240 + 290 * std::sqrt(i.phase / 40.f);
                i.o->position = {int(r * std::cos(a)),
                                 -350 + int(90 * std::sin(t * 1.2f + i.phase)),
                                 int(r * std::sin(a))};
                i.o->rotation = {int(18 * std::sin(a)), int(i.phase * 35 + t * 42),
                                 int(20 * std::cos(a))};
            } else if (i.kind == 1) {
                i.o->rotation = {17, int(t * 44), 12};
                i.o->position.y = int(60 + 50 * std::sin(t));
            } else {
                i.o->rotation = {int(i.phase * 50 + t * 34), int(t * 43), int(i.phase * 28)};
            }
        }
    }
    if (chapter == 8) {
        camera.setFOV(76.f, width);
        camera.setPosition({int(55 * std::sin(t * .5f)), int(45 * std::cos(t * .4f)), -520});
        camera.lookAt({0, 0, 2500});
        for (auto &i : items) {
            if (i.kind == 0) {
                float z = std::fmod(i.phase * 260 - t * 360 + 3120, 3120.f);
                if (z < 0)
                    z += 3120;
                i.o->position = {int(40 * std::sin(z * .0025f + t * .5f)),
                                 int(40 * std::cos(z * .0025f + t * .5f)), int(z) - 300};
                i.o->rotation.z = int(i.phase * 13 + t * 42 + 8 * std::sin(t * 1.3f + i.phase));
            } else {
                // A foreground sculpture inside the clear bore, rather than a
                // tiny mark at the vanishing point. Tunnel profiles pass around it.
                i.o->position.z = 500;
                i.o->rotation = {int(t * 38), int(t * 51), 0};
            }
        }
    }
    if (chapter == 9) {
        orbit(-.4f + t * .055f, 1100 + 12 * t, 250 + 8 * t, {0, -10, 0});
        for (auto &i : items) {
            if (i.kind == 0) {
                i.o->rotation = {int(t * 38), int(t * 47), 10};
            } else if (i.kind == 1) {
                float a = t * .36f + i.phase * tau / 12,
                      r = 400 + 75 * std::sin(t * .45f + i.phase);
                i.o->position = {int(r * std::cos(a)), int(160 * std::sin(a * 2 + t * .25f)),
                                 int(r * std::sin(a))};
                i.o->rotation = {int(t * 46 + i.phase * 21), int(t * 52 + i.phase * 30),
                                 int(t * 39)};
            } else {
                i.o->rotation = {int(i.phase * 50 + t * 34), int(i.phase * 40 - t * 41),
                                 int(t * 37)};
            }
        }
    }
    for (auto *o : reflective)
        envMap(o);
    projectGlows(t, chapter);
}
inline void seek(float absolute) {
    time = std::clamp(absolute, 0.f, duration);
    int next = std::min(chapterCount - 1, int(time / 18));
    local = time - next * 18;
    if (next != chapter)
        load(next);
    pose(local);
    // Brief dark apertures conceal scene replacement without repeated flashes.
    float alpha = chapter == 0 ? 1 - clamp(local / 2.2f) : 1 - clamp(local / .55f);
    if (chapter == 9)
        alpha = std::max(alpha, clamp((local - 15) / 3));
    else
        alpha = std::max(alpha, clamp((local - 17.5f) / .5f));
    setSolidRectAlpha(fade, uint8_t(255 * alpha));
    if (chapter == 0)
        lettering(0, 28, 34,
                  int(255 * std::min(clamp((local - 1.5f)), 1 - clamp((local - 6) / 1.5f))));
    else if (chapter == 9)
        lettering(11, 26, 201, int(255 * clamp((local - 6) / 1.5f)));
    else
        lettering(
            chapter + 1, 22, 278,
            int(230 * std::min(clamp((local - .65f) / .6f), 1 - clamp((local - 3.5f) / .8f))));
}
inline void update(float dt) {
    float next = time + dt;
    if (next >= duration + 1) {
        finished = true;
#if defined(ESP_PLATFORM)
        std::fprintf(stderr, "MATTER COMPLETE / REBOOT\n");
        esp_restart();
#else
        seek(duration);
        return;
#endif
    }
    seek(next);
    if (next > duration)
        time = next;
}
inline unsigned effects(Scene &) {
    return 0;
}
inline void init(Scene &target) {
    scene = &target;
    scene->setCamera(&camera);
    scene->setClearBuffer(true);
    scene->setDirectionalLight(&light);
    scene->setAmbientLight(&ambient);
    scene->backgroundGradientColors = sky;
    textures();
    typography();
    fade = makeFullScreenFade(width, height, 0, &fadeMaterial);
    fade.zOrder = 900000;
    scene->addSprite(&fade);
    seek(0);
}
} // namespace Matter
