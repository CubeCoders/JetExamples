#pragma once
#include "Scene.hpp"
#include "Display.hpp"
#include "TeapotMesh.hpp"
#include "Labels.hpp"
#include <cmath>
#include <cstdio>

namespace CelTeapot {
inline Renderer::Camera camera;
inline Renderer::DirectionalLight key({235, 35, 0}, {255, 244, 224}, 255);
inline Renderer::AmbientLight ambient({30, 40, 56});
inline Renderer::Material glaze(0xD8C3, nullptr, nullptr, false, 255, 210, 255);
inline Renderer::Object* mesh = nullptr;
inline uint16_t background[Display::RENDER_HEIGHT];
inline float time = 0;
inline int activeMode = -1;
inline Renderer::Scene* targetScene = nullptr;
inline void selectMode(int mode) {
    if (activeMode == mode) return;
    activeMode = mode;
    targetScene->getRenderer()->celShadingEnabled = mode == 1;
    Labels::select(mode);
    std::printf("Cel shading: %s; four diffuse bands; additive gloss unchanged\n",mode ? "ON" : "OFF");
}

inline void init(Renderer::Scene& scene) {
    targetScene = &scene;
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
    std::printf("Utah teapot: %u vertices, %u triangles, fixed Phong + depth, smooth / cel cycle\n",
                unsigned(mesh->vertices.size()), unsigned(mesh->triangles.size()));
}

inline void update(float seconds) {
    time = std::fmod(time + seconds, 14.0f);
    selectMode(int(time / 7.0f) % 2);
    // Each seven-second phase follows exactly the same seamless motion loop.
    // Only lighting quantisation changes; geometry, material and light stay fixed.
    const float phase = std::fmod(time, 7.0f) / 7.0f;
    const float pitch = -14.0f + 20.0f * std::sin(phase * 6.283185307f);
    const float roll = 10.0f * std::sin(phase * 12.566370614f);
    mesh->setRotation(int(pitch), int(25.0f + 360.0f * phase) % 360, int(roll));
}
}
