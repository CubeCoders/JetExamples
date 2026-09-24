#pragma once
#include "Labels.hpp"
#include "Scene.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>
#if defined(ESP_PLATFORM)
#include <esp_heap_caps.h>
#endif

// One immutable mesh, independently positioned and coloured instances.
// Both modes use the same engine capabilities, lighting and camera motion.
namespace Instancing {
using namespace Renderer;
inline constexpr int count = 15, columns = 5;
inline constexpr float secondsPerMode = 8.f, pi = 3.14159265358979323846f;
inline Scene *scene = nullptr;
inline Camera camera;
inline DirectionalLight key({225, 35, 0}, {255, 244, 225}, 235);
inline AmbientLight ambient({76, 83, 95});
inline std::array<Material, 5> paints;
inline std::vector<std::unique_ptr<Object>> objects;
inline Object::SharedMesh prototype;
inline uint16_t sky[320];
inline float time = 0;
inline int activeMode = -1;
inline size_t meshBytes = 0, meshVertices = 0, meshTriangles = 0;
inline constexpr uint16_t rgb(unsigned c) {
    return uint16_t(((c >> 19) & 31) << 11 | ((c >> 10) & 63) << 5 | ((c >> 3) & 31));
}
struct V {
    float x, y, z;
    V operator-(V b) const { return {x - b.x, y - b.y, z - b.z}; }
    V cross(V b) const { return {y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x}; }
    Vector3 integer(float k = 1) const {
        return {int(std::round(x * k)), int(std::round(y * k)), int(std::round(z * k))};
    }
};
inline void quad(Object &o, V a, V b, V c, V d, Material *material) {
    auto n = (b - a).cross(c - a);
    float length = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
    auto normal = n.integer(1024.f / length);
    int first = int(o.vertices.size());
    for (auto p : {a, b, c, d})
        o.addVertex({p.integer(), {0, 0}, normal});
    o.addFace(first, first + 1, first + 2, first + 3, material);
}
inline Object makeRotor() {
    Object o;
    constexpr int segments = 20;
    o.vertices.reserve(segments * 5 * 4);
    o.triangles.reserve(segments * 5 * 2);
    auto point = [&](int i, int profile) {
        float a = i * 2 * pi / segments;
        float outer = i % 2 ? 58.f : 65.f;
        const float radius[] = {25.f, outer - 8, outer, outer - 8, 25.f};
        const float z[] = {-16, -16, 0, 16, 16};
        return V{radius[profile] * std::cos(a), radius[profile] * std::sin(a), z[profile]};
    };
    for (int i = 0; i < segments; ++i)
        for (int p = 0; p < 5; ++p)
            quad(o, point(i, p), point(i + 1, p), point(i + 1, (p + 1) % 5), point(i, (p + 1) % 5),
                 &paints[0]);
    o.calculateBoundingBox();
    o.cachePositions();
    return o;
}
// Stored geometry payload, including Jet's packed positions and reuse map.
// It excludes Object/instance metadata, allocator overhead and frame buffers.
inline size_t storage(const Object &o) {
    return o.vertices.capacity() * sizeof(Object::Vertex) +
           o.triangles.capacity() * sizeof(Object::Triangle) + o.indices.capacity() * sizeof(int) +
           (o.cachedPositions() ? o.vertices.size() * sizeof(Vector3) : 0) +
           (o.cachedPositionSources() ? o.vertices.size() * sizeof(uint16_t) : 0);
}
inline void clearMeshes() {
    scene->getObjects().clear();
    objects.clear();
    prototype.reset();
}
inline void selectMode(int mode) {
    if (mode == activeMode)
        return;
    clearMeshes();
    activeMode = mode;
#if defined(ESP_PLATFORM) && defined(CONFIG_SPIRAM)
    heap_caps_malloc_extmem_enable(128);
#endif
    {
        auto authored = makeRotor();
        meshVertices = authored.vertices.size();
        meshTriangles = authored.triangles.size();
        meshBytes = 0;
        if (mode) {
            prototype = Object::freezeMesh(std::move(authored));
            meshBytes = storage(*prototype);
        }
        for (int i = 0; i < count; ++i) {
            std::unique_ptr<Object> o;
            if (mode) {
                o = std::make_unique<Object>();
                // Identity placement; the owner provides the live pose. Materials
                // are borrowed from paints, whose lifetime exceeds every mesh.
                if (!o->addInstance(prototype, {}, &paints[i % paints.size()]))
                    std::abort();
                o->calculateBoundingBox();
            } else {
                o = std::make_unique<Object>(authored);
                // Object copies normally share their packed position cache.
                // Rebuild it here to demonstrate fully independent mesh copies.
                o->invalidatePositions();
                o->cachePositions();
                for (auto &triangle : o->triangles)
                    triangle.material = &paints[i % paints.size()];
                meshBytes += storage(*o);
            }
            o->preciseDepthSort = true;
            scene->addObject(o.get());
            objects.push_back(std::move(o));
        }
    } // The temporary authoring mesh is gone before reporting memory.
#if defined(ESP_PLATFORM) && defined(CONFIG_SPIRAM)
    heap_caps_malloc_extmem_enable(CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL);
#endif
    Labels::select(mode, count, meshBytes);
    std::printf("INSTANCING mode=%s objects=%d stored_meshes=%d mesh_bytes=%u logical_tris=%u\n",
                mode ? "shared" : "copies", count, mode ? 1 : count, unsigned(meshBytes),
                unsigned(count * meshTriangles));
#if defined(ESP_PLATFORM)
    std::printf("INSTANCING heap psram_used=%u internal_free=%u\n",
                unsigned(heap_caps_get_total_size(MALLOC_CAP_SPIRAM) -
                         heap_caps_get_free_size(MALLOC_CAP_SPIRAM)),
                unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)));
#endif
}
inline void pose(float local) {
    // Two full turns per mode: quick enough to hide whole-degree quantisation,
    // with no jump when the memory strategy switches and the motion repeats.
    for (int i = 0; i < count; ++i) {
        objects[i]->setPosition((i % columns - 2) * 145, (1 - i / columns) * 130, (i % 3 - 1) * 20);
        objects[i]->setRotation(-18 + (i / columns) * 12, (i % columns - 2) * 11,
                                (int(std::round(local * 90)) + i * 18) % 360);
    }
    Labels::progress.width = 444 * std::fmod(local, secondsPerMode) / secondsPerMode;
}
inline void seek(float absolute) {
    time = absolute;
    selectMode(int(time / secondsPerMode) % 2);
    pose(std::fmod(time, secondsPerMode));
}
inline void update(float dt) { seek(std::fmod(time + dt, secondsPerMode * 2)); }
inline void init(Scene &target) {
    scene = &target;
    camera.setPosition(0, 0, -880);
    camera.setRotation(0, 0, 0);
    camera.setFOV(57.f, 480);
    camera.nearPlane = 32;
    camera.farPlane = 3000;
    scene->setCamera(&camera);
    scene->setClearBuffer(true);
    scene->setDirectionalLight(&key);
    scene->setAmbientLight(&ambient);
    const unsigned colours[] = {0xDF805E, 0xEAC77D, 0xB7D4C2, 0x70B7B6, 0xB2B8DA};
    for (int i = 0; i < 5; ++i) {
        paints[i].color = rgb(colours[i]);
        paints[i].shadingMode = ShadingMode::PHONG;
        paints[i].specular = 150;
        paints[i].specularExponent = 24;
    }
    for (int y = 0; y < 320; ++y)
        sky[y] = rgb(((22 + y / 9) << 16) | ((35 + y / 7) << 8) | (49 + y / 6));
    scene->backgroundGradientColors = sky;
#if defined(ESP_PLATFORM) && defined(CONFIG_SPIRAM)
    heap_caps_malloc_extmem_enable(128);
#endif
    objects.reserve(count);
    Labels::init(target);
#if defined(ESP_PLATFORM) && defined(CONFIG_SPIRAM)
    heap_caps_malloc_extmem_enable(CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL);
#endif
    seek(0);
}
} // namespace Instancing
