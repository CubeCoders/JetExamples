#pragma once
#include "Geometry.hpp"
namespace Matter {
// Small immutable procedural textures are shared across every movement.
// Their lifetime also covers asynchronous display scanout of sprite pixels.
inline std::vector<uint16_t> roomPixels, glowPixels;
inline Texture room(128, 64, nullptr), halo(32, 32, nullptr, true, 0);
inline void textures() {
    if (!roomPixels.empty())
        return;
    roomPixels.resize(8192);
    glowPixels.resize(1024);
    for (int y = 0; y < 64; ++y)
        for (int x = 0; x < 128; ++x) {
            float v = y / 64.f;
            unsigned c = colour(0x22312F, 0xDED8C3, .5f + .5f * std::cos(v * pi));
            if ((x > 13 && x < 28) || (x > 73 && x < 93))
                c = colour(0xBACCCC, 0xFFFCDF, clamp(1 - std::abs(v - .32f) * 2));
            if (x > 34 && x < 47)
                c = colour(0x713D24, 0xDC7E36, 1 - v);
            if (y > 36 && y < 41)
                c = 0x151B22;
            if ((x % 32) < 3)
                c = 0x3B4543;
            roomPixels[y * 128 + x] = rgb(c);
        }
    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 32; ++x) {
            float dx = (x - 15.5f) / 15.5f, dy = (y - 15.5f) / 15.5f,
                  v = std::max(0.f, 1 - std::sqrt(dx * dx + dy * dy));
            v = v * v * v;
            glowPixels[y * 32 + x] =
                rgb((unsigned(255 * v) << 16) | (unsigned(207 * v) << 8) | unsigned(135 * v));
        }
    room.data = roomPixels.data();
    halo.data = glowPixels.data();
    room.bilinear = false;
    halo.bilinear = false;
}
struct ParticleGlow {
    Sprite2D sprite;
    Material material;
    V position;
};
inline std::vector<std::unique_ptr<ParticleGlow>> glows;
inline void clearGlows() {
    auto &s = scene->getSprites();
    for (auto &g : glows)
        s.erase(std::remove(s.begin(), s.end(), &g->sprite), s.end());
    glows.clear();
}
inline void addGlow(V position) {
    glows.emplace_back(new ParticleGlow);
    auto &g = *glows.back();
    g.position = position;
    g.material.diffuseMap = &halo;
    g.material.shadingMode = ShadingMode::UNLIT;
    g.sprite.material = &g.material;
    g.sprite.blendMode = BlendMode::BLEND_ADD;
    g.sprite.zOrder = 10;
    scene->addSprite(&g.sprite);
}
inline void projectGlows(float t, int chapter) {
    for (size_t i = 0; i < glows.size(); ++i) {
        auto &g = *glows[i];
        V p = g.position;
        if (chapter == 5) {
            float a = t * .5f + i * 2.39996f;
            p = {std::cos(a) * (210 + i * 12), -190 + std::fmod(t * 65 + i * 51, 600.f),
                 std::sin(a) * (210 + i * 12)};
        }
        auto v = camera.transformDirection(p.integer() - camera.position);
        g.sprite.enabled = v.z > 120;
        if (!g.sprite.enabled)
            continue;
        g.sprite.scale =
            std::clamp(int(90 * camera.fovFactor / (v.z * 32)), 1 * renderScale, 3 * renderScale);
        g.sprite.x = width / 2 + int(v.x * camera.fovFactor / v.z) - 16 * g.sprite.scale;
        g.sprite.y = height / 2 - int(v.y * camera.fovFactor / v.z) - 16 * g.sprite.scale;
    }
}
// Analytic radial deformation with the tangent gradient included in normals.
// A single wavelength sample per axis yields a lit liquid, not an unlit wobble.
struct Blob {
    Object *object = nullptr;
    struct Basis {
        V direction, sine, cosine;
    };
    std::vector<Basis> directions;
    float radius = 240;
    void build(Material *m, int seg = 24) {
        object = sphere(int(radius), seg, m);
        object->invalidatePositions();
        for (auto &v : object->vertices) {
            V n = vector(v.position).unit();
            directions.push_back({n,
                                  {std::sin(n.x * 4), std::sin(n.y * 5), std::sin(n.z * 4)},
                                  {std::cos(n.x * 4), std::cos(n.y * 5), std::cos(n.z * 4)}});
        }
        object->boundingBoxMin = {-400, -400, -400};
        object->boundingBoxMax = {400, 400, 400};
    }
    void pose(float t) {
        // Six trig evaluations per frame instead of six per vertex.
        V st{std::sin(t * 1.1f), std::sin(-t * .8f), std::sin(t * .9f)};
        V ct{std::cos(t * 1.1f), std::cos(-t * .8f), std::cos(t * .9f)};
        for (size_t i = 0; i < directions.size(); ++i) {
            auto &basis = directions[i];
            V n = basis.direction;
            float sx = basis.sine.x * ct.x + basis.cosine.x * st.x;
            float sy = basis.sine.y * ct.y + basis.cosine.y * st.y;
            float sz = basis.sine.z * ct.z + basis.cosine.z * st.z;
            float cx = basis.cosine.x * ct.x - basis.sine.x * st.x;
            float cy = basis.cosine.y * ct.y - basis.sine.y * st.y;
            float cz = basis.cosine.z * ct.z - basis.sine.z * st.z;
            float r = radius + 35 * sx * sy * sz;
            V grad{140 * cx * sy * sz, 175 * sx * cy * sz, 140 * sx * sy * cz};
            float dot = grad.x * n.x + grad.y * n.y + grad.z * n.z;
            V normal = (n * r - grad + n * dot).unit();
            object->vertices[i].position = (n * r).integer();
            object->vertices[i].normal = normal.integer(1024);
        }
    }
    void clear() {
        object = nullptr;
        directions.clear();
    }
};
inline Blob blob;
struct Ribbon {
    Object *object;
    int samples;
    float length, width, phase;
};
inline std::vector<Ribbon> ribbons;
inline void ribbon(int samples, float length, float w, float phase, Material *a, Material *b) {
    auto *o = bank.mesh();
    o->cullingMode = CullingMode::NO_CULLING;
    for (int i = 0; i <= samples; ++i)
        for (int j = 0; j < 3; ++j)
            o->addVertex({{0, 0, 0}});
    for (int i = 0; i < samples; ++i)
        for (int j = 0; j < 2; ++j) {
            int n = i * 3 + j;
            o->addFace(n, n + 3, n + 4, n + 1, (i / 3) % 2 ? a : b);
        }
    bank.finish(o);
    o->invalidatePositions();
    o->boundingBoxMin = {-1000, -1000, -int(length)};
    o->boundingBoxMax = {1000, 1000, int(length)};
    ribbons.push_back({o, samples, length, w, phase});
}
inline void poseRibbons(float t) {
    for (auto &r : ribbons) {
        for (int i = 0; i <= r.samples; ++i) {
            float u = float(i) / r.samples, z = (u - .5f) * r.length, a = u * 9 + t * .7f + r.phase;
            V c{300 * std::cos(a), 270 * std::sin(a), z};
            V side{std::cos(a * 1.1f), std::sin(a * 1.1f), 0};
            V tangent{-2700 / r.length * std::sin(a), 2430 / r.length * std::cos(a), 1};
            V normal = side.cross(tangent).unit();
            for (int j = 0; j < 3; ++j) {
                auto &v = r.object->vertices[i * 3 + j];
                v.position = (c + side * ((j - 1) * r.width)).integer();
                v.normal = normal.integer(1024);
            }
        }
    }
}
} // namespace Matter
