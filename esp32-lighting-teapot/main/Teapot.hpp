#pragma once
#include "Scene.hpp"
#include "Display.hpp"
#include "TeapotMesh.hpp"
#include "Labels.hpp"
#include <cmath>
#include <cstdio>

namespace Teapot {
inline Renderer::Camera camera;
inline Renderer::DirectionalLight key({235, 35, 0}, {255, 244, 224}, 255);
inline Renderer::AmbientLight ambient({30, 40, 56});
inline Renderer::Material glaze(0xD8C3, nullptr, nullptr, false, 255, 210, 255);
inline Renderer::Object* mesh = nullptr;
inline uint16_t background[Display::RENDER_HEIGHT];
inline float time = 0;
inline int activeMode = -1;
inline void selectMode(int mode) {
    if (activeMode == mode) return;
    activeMode = mode;
    constexpr Renderer::ShadingMode modes[] = {Renderer::ShadingMode::FLAT,
        Renderer::ShadingMode::GOURAUD, Renderer::ShadingMode::PHONG};
    constexpr const char* names[] = {"FLAT", "GOURAUD", "PHONG"};
    glaze.shadingMode = modes[mode];
    Labels::select(mode);
    std::printf("Shading mode: %s\n", names[mode]);
}

inline void init(Renderer::Scene& scene) {
    camera.setPosition(0, 0, -1250);
    camera.setFOV(60.0f, Display::RENDER_WIDTH);
    camera.nearPlane = 32;
    camera.farPlane = 3000;
    scene.setCamera(&camera);
    scene.setDirectionalLight(&key);
    scene.setAmbientLight(&ambient);
    scene.setClearBuffer(true);
    for (int y = 0; y < Display::RENDER_HEIGHT; ++y) {
        const int r = 12 + y * 24 / Display::RENDER_HEIGHT;
        const int g = 28 + y * 56 / Display::RENDER_HEIGHT;
        const int b = 54 + y * 72 / Display::RENDER_HEIGHT;
        background[y] = uint16_t((r >> 3) << 11 | (g >> 2) << 5 | (b >> 3));
    }
    scene.backgroundGradientColors = background;
    glaze.shadingMode = Renderer::ShadingMode::PHONG;
    glaze.specularExponent = 32;
    mesh = TeapotMesh::create(&glaze);
    mesh->setRotation(-14, 25, -4);
    scene.addObject(mesh);
    Labels::add(scene);
    selectMode(0);
    std::printf("Utah teapot: %u vertices, %u triangles, Flat / Gouraud / Phong cycle\n",
                unsigned(mesh->vertices.size()), unsigned(mesh->triangles.size()));
}

inline void update(float seconds) {
    time = std::fmod(time + seconds, 3600.0f);
    selectMode(int(time / 3.0f) % 3);
    // Faster rocking avoids lingering on integer-degree steps. Keep the
    // familiar silhouette visible as all three axes move beneath the light.
    const float pitch = -14.0f + 20.0f * std::sin(time * 1.8f);
    const float roll = 10.0f * std::sin(time * 1.4f);
    mesh->setRotation(int(pitch), int(std::fmod(25.0f + time * 25.0f, 360.0f)), int(roll));
}
}
