#pragma once
#include "Scene.hpp"
#include "Primitives.hpp"
#include "EnvironmentMapping.hpp"
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace Matter {
using namespace Renderer;
#ifndef MATTER_RENDER_SCALE
#define MATTER_RENDER_SCALE 1
#endif
inline constexpr int renderScale = MATTER_RENDER_SCALE;
inline constexpr int width = 480 * renderScale, height = 320 * renderScale;
inline constexpr float pi = 3.14159265358979323846f, tau = 2 * pi;
inline Scene *scene = nullptr;
inline Camera camera;
inline DirectionalLight light({225, 40, 0}, {255, 240, 214}, 220);
inline AmbientLight ambient({92, 93, 99});
inline uint16_t sky[height];
inline uint16_t rgb(unsigned c) {
    return uint16_t(((c >> 19) & 31) << 11 | ((c >> 10) & 63) << 5 | ((c >> 3) & 31));
}
inline float clamp(float a) {
    return std::clamp(a, 0.f, 1.f);
}
inline float mix(float a, float b, float t) {
    return a + (b - a) * t;
}
inline Vector3 lerp(Vector3 a, Vector3 b, float t) {
    return {int(mix(float(a.x), float(b.x), t)), int(mix(float(a.y), float(b.y), t)),
            int(mix(float(a.z), float(b.z), t))};
}
inline unsigned colour(unsigned a, unsigned b, float t) {
    unsigned c = 0;
    for (int s : {0, 8, 16})
        c |= unsigned(mix(float((a >> s) & 255), float((b >> s) & 255), clamp(t))) << s;
    return c;
}
inline uint32_t hash(uint32_t a) {
    a ^= a >> 16;
    a *= 0x7feb352du;
    a ^= a >> 15;
    a *= 0x846ca68bu;
    return a ^ (a >> 16);
}
inline float randf(unsigned i) {
    return (hash(i) & 65535) / 65535.f;
}
struct V {
    float x = 0, y = 0, z = 0;
    V operator+(V b) const {
        return {x + b.x, y + b.y, z + b.z};
    }
    V operator-(V b) const {
        return {x - b.x, y - b.y, z - b.z};
    }
    V operator*(float k) const {
        return {x * k, y * k, z * k};
    }
    V cross(V b) const {
        return {y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x};
    }
    V unit() const {
        float l = std::sqrt(x * x + y * y + z * z);
        return *this * (1 / std::max(l, .0001f));
    }
    Vector3 integer(float scale = 1) const {
        return {int(std::round(x * scale)), int(std::round(y * scale)), int(std::round(z * scale))};
    }
};
inline V vector(Vector3 p) {
    return {float(p.x), float(p.y), float(p.z)};
}
inline V rotated(V v, Vector3 r) {
    float x = r.x * pi / 180, y = r.y * pi / 180, z = r.z * pi / 180;
    v = {v.x, v.y * std::cos(x) - v.z * std::sin(x), v.y * std::sin(x) + v.z * std::cos(x)};
    v = {v.x * std::cos(y) + v.z * std::sin(y), v.y, -v.x * std::sin(y) + v.z * std::cos(y)};
    return {v.x * std::cos(z) - v.y * std::sin(z), v.x * std::sin(z) + v.y * std::cos(z), v.z};
}
struct Bank {
    std::vector<std::unique_ptr<Object>> objects;
    std::vector<std::unique_ptr<Material>> materials;
    void clear() {
        scene->getObjects().clear();
        objects.clear();
        materials.clear();
    }
    Material *paint(unsigned c, ShadingMode mode = ShadingMode::GOURAUD, int alpha = 255) {
        materials.emplace_back(new Material(rgb(c)));
        auto *m = materials.back().get();
        m->shadingMode = mode;
        m->alpha = uint8_t(alpha);
        return m;
    }
    Material *texture(Texture *t, ShadingMode mode = ShadingMode::UNLIT) {
        auto *m = paint(0xffffff, mode);
        m->diffuseMap = t;
        m->perspectiveCorrect = false;
        return m;
    }
    Object *own(Object *o) {
        objects.emplace_back(o);
        return o;
    }
    Object *mesh() {
        auto *o = own(new Object);
        o->preciseDepthSort = true;
        return o;
    }
    Object *finish(Object *o, bool background = false) {
        o->calculateBoundingBox();
        o->cachePositions();
        o->noWriteZBuffer = background;
        scene->addObject(o);
        return o;
    }
};
inline Bank bank;
inline void tri(Object *o, V a, V b, V c, Material *m) {
    V n = (b - a).cross(c - a).unit();
    int k = int(o->vertices.size());
    o->addVertex({a.integer(), {0, 0}, n.integer(1024)});
    o->addVertex({b.integer(), {1024, 0}, n.integer(1024)});
    o->addVertex({c.integer(), {512, 1024}, n.integer(1024)});
    o->addTriangle(k, k + 1, k + 2, m);
}
inline void quad(Object *o, V a, V b, V c, V d, Material *m) {
    V n = (b - a).cross(c - a).unit();
    int k = int(o->vertices.size());
    o->addVertex({a.integer(), {0, 0}, n.integer(1024)});
    o->addVertex({b.integer(), {1024, 0}, n.integer(1024)});
    o->addVertex({c.integer(), {1024, 1024}, n.integer(1024)});
    o->addVertex({d.integer(), {0, 1024}, n.integer(1024)});
    o->addFace(k, k + 1, k + 2, k + 3, m);
}
inline void flatNormals(Object *o) {
    for (auto &t : o->triangles) {
        auto &a = o->vertices[t.v1];
        auto &b = o->vertices[t.v2];
        auto &c = o->vertices[t.v3];
        auto n = (vector(b.position) - vector(a.position))
                     .cross(vector(c.position) - vector(a.position))
                     .unit()
                     .integer(1024);
        a.normal = b.normal = c.normal = n;
    }
}
inline Object *box(Vector3 p, Vector3 size, Material *m) {
    auto *o = bank.own(Primitives::createCube(size.x, size.y, size.z, m));
    o->position = p;
    o->preciseDepthSort = true;
    return bank.finish(o);
}
inline Object *sphere(int r, int seg, Material *m, Vector3 p = {}) {
    auto *o = bank.own(Primitives::createSphere(r, seg, m));
    o->position = p;
    o->preciseDepthSort = true;
    return bank.finish(o);
}
inline Object *cylinder(int r, int h, int seg, Material *m, Vector3 p = {}) {
    auto *o = bank.mesh();
    for (int i = 0; i < seg; ++i) {
        float a = tau * i / seg, b = tau * (i + 1) / seg;
        V v{r * std::cos(a), h * .5f, r * std::sin(a)},
            w{r * std::cos(b), h * .5f, r * std::sin(b)}, vv{v.x, -h * .5f, v.z},
            ww{w.x, -h * .5f, w.z};
        quad(o, v, w, ww, vv, m);
        tri(o, {0, h * .5f, 0}, w, v, m);
        tri(o, {0, -h * .5f, 0}, vv, ww, m);
    }
    o->position = p;
    return bank.finish(o);
}
inline void gradient(unsigned top, unsigned bottom) {
    for (int y = 0; y < height; ++y)
        sky[y] = rgb(colour(top, bottom, float(y) / (height - 1)));
}
inline Object *floor(Material *a, Material *b, int extent = 2600, int cells = 10, int y = -420) {
    auto *o = bank.own(Primitives::createGrid(extent, extent, cells, cells, a, b));
    o->position.y = y;
    return bank.finish(o, true);
}
inline Object *ring(float major, float minor, int around, int sides, Material *material,
                    Vector3 p = {}) {
    auto *o = bank.mesh();
    for (int i = 0; i <= around; ++i) {
        float u = tau * i / around;
        for (int j = 0; j <= sides; ++j) {
            float v = tau * j / sides;
            V n{std::cos(u) * std::cos(v), std::sin(v), std::sin(u) * std::cos(v)};
            V q{std::cos(u) * (major + minor * std::cos(v)), minor * std::sin(v),
                std::sin(u) * (major + minor * std::cos(v))};
            o->addVertex({q.integer(), {i * 1024 / around, j * 1024 / sides}, n.integer(1024)});
        }
    }
    for (int i = 0; i < around; ++i)
        for (int j = 0; j < sides; ++j) {
            int a = i * (sides + 1) + j, b = a + sides + 1;
            o->addFace(a, a + 1, b + 1, b, material);
        }
    o->position = p;
    return bank.finish(o);
}
inline Object *knot(float radius, float tube, int segments, int sides, Material *material) {
    auto *o = bank.mesh();
    auto path = [&](float t) {
        return V{radius * (2 + std::cos(3 * t)) * .5f * std::cos(2 * t),
                 radius * .5f * std::sin(3 * t),
                 radius * (2 + std::cos(3 * t)) * .5f * std::sin(2 * t)};
    };
    for (int i = 0; i <= segments; ++i) {
        float u = tau * i / segments;
        V c = path(u), t = (path(u + .001f) - path(u - .001f)).unit(),
          n = t.cross({0, 1, 0}).unit(), b = t.cross(n).unit();
        for (int j = 0; j <= sides; ++j) {
            float v = tau * j / sides;
            V normal = n * std::cos(v) + b * std::sin(v);
            o->addVertex({(c + normal * tube).integer(),
                          {i * 4096 / segments, j * 1024 / sides},
                          normal.integer(1024)});
        }
    }
    for (int i = 0; i < segments; ++i)
        for (int j = 0; j < sides; ++j) {
            int a = i * (sides + 1) + j, b = a + sides + 1;
            o->addFace(a, b, b + 1, a + 1, material);
        }
    return bank.finish(o);
}
// Variable-depth industrial annulus. Two rounded bevel bands have continuous
// normals across both their profile and the tooth outline, while the broad
// front/back faces and tooth walls stay flat. Phong catches the curved lip.
inline Object *gear(int radius, int teeth, Material *front, Material *edge) {
    auto *o = bank.mesh();
    int n = teeth * 4;
    float inner = radius * .43f;
    auto pt = [&](int i, float r, float z) {
        float a = tau * i / n;
        return V{r * std::cos(a), r * std::sin(a), z};
    };
    auto outer = [&](int i) { return radius * (((i % n + n) % n % 4 < 2) ? 1.f : .89f); };
    auto radial = [&](int i) {
        V previous = pt(i, outer(i), 0) - pt(i - 1, outer(i - 1), 0);
        V next = pt(i + 1, outer(i + 1), 0) - pt(i, outer(i), 0);
        return (V{previous.y, -previous.x, 0}.unit() + V{next.y, -next.x, 0}.unit()).unit();
    };
    for (int i = 0; i < n; ++i) {
        float a = outer(i), b = outer(i + 1);
        quad(o, pt(i, inner, 26), pt(i, a - 14, 26), pt(i + 1, b - 14, 26), pt(i + 1, inner, 26),
             front);
        for (int band = 0; band < 2; ++band) {
            float u = band * pi / 4, v = (band + 1) * pi / 4;
            int k = int(o->vertices.size());
            quad(o, pt(i, a - 14 + 14 * std::sin(u), 12 + 14 * std::cos(u)),
                 pt(i, a - 14 + 14 * std::sin(v), 12 + 14 * std::cos(v)),
                 pt(i + 1, b - 14 + 14 * std::sin(v), 12 + 14 * std::cos(v)),
                 pt(i + 1, b - 14 + 14 * std::sin(u), 12 + 14 * std::cos(u)), edge);
            o->vertices[k].normal = (radial(i) * std::sin(u) + V{0, 0, std::cos(u)}).integer(1024);
            o->vertices[k + 1].normal =
                (radial(i) * std::sin(v) + V{0, 0, std::cos(v)}).integer(1024);
            o->vertices[k + 2].normal =
                (radial(i + 1) * std::sin(v) + V{0, 0, std::cos(v)}).integer(1024);
            o->vertices[k + 3].normal =
                (radial(i + 1) * std::sin(u) + V{0, 0, std::cos(u)}).integer(1024);
        }
        quad(o, pt(i, a, 12), pt(i, a, -26), pt(i + 1, b, -26), pt(i + 1, b, 12), edge);
        quad(o, pt(i + 1, inner, -26), pt(i + 1, b, -26), pt(i, a, -26), pt(i, inner, -26), front);
        quad(o, pt(i + 1, inner, 26), pt(i + 1, inner, -26), pt(i, inner, -26), pt(i, inner, 26),
             edge);
    }
    return bank.finish(o);
}
inline Object *crystal(float r, float length, int sides, Material *a, Material *b) {
    auto *o = bank.mesh();
    for (int i = 0; i < sides; ++i) {
        float u = tau * i / sides, v = tau * (i + 1) / sides;
        V p{r * std::cos(u), 0, r * std::sin(u)}, q{r * std::cos(v), 0, r * std::sin(v)};
        tri(o, p, {0, length, 0}, q, i % 2 ? a : b);
        tri(o, q, {0, -length * .35f, 0}, p, b);
    }
    return bank.finish(o);
}
inline void aim(Object *o, V a, V b) {
    V d = b - a;
    float len = std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
    o->position = ((a + b) * .5f).integer();
    o->rotation = {int(std::atan2(std::sqrt(d.x * d.x + d.z * d.z), d.y) * 180 / pi),
                   int(std::atan2(d.x, d.z) * 180 / pi), 0};
    o->scale = {1024, int(len * 1024 / 100), 1024};
    o->transformScale = true;
}
inline void envMap(Object *o) {
    V xx = rotated({1, 0, 0}, o->rotation), yy = rotated({0, 1, 0}, o->rotation),
      zz = rotated({0, 0, 1}, o->rotation);
    auto turn = [&](Vector3 p) { return xx * float(p.x) + yy * float(p.y) + zz * float(p.z); };
    for (auto &v : o->vertices) {
        V p = turn(v.position) + vector(o->position), n = turn(v.normal);
        v.uv = environmentReflectionUV(p.integer(), n.integer(), camera.position);
    }
}
inline void orbit(float a, float radius, float y, Vector3 target = {}) {
    camera.setPosition({target.x + int(std::sin(a) * radius), target.y + int(y),
                        target.z + int(std::cos(a) * radius)});
    camera.lookAt(target);
}
} // namespace Matter
