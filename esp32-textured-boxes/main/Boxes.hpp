#pragma once
#include "Scene.hpp"
#include "Primitives.hpp"
#include "Display.hpp"
#include "Labels.hpp"
#include "CrateTexture.hpp"
#include <cmath>
#include <cstring>
#if defined(ESP_PLATFORM)
#include <esp_heap_caps.h>
#endif

namespace Boxes {
inline Renderer::Camera camera;
inline Renderer::DirectionalLight light({235,35,0},{255,244,230},255);
inline Renderer::AmbientLight ambient({80,90,104});
inline Renderer::Texture textures[4]={
    {CrateTexture::width,CrateTexture::height,const_cast<uint16_t*>(CrateTexture::pixels)},
    {CrateTexture::width,CrateTexture::height,const_cast<uint16_t*>(CrateTexture::pixels)},
    {CrateTexture::width,CrateTexture::height,const_cast<uint16_t*>(CrateTexture::pixels)},
    {CrateTexture::width,CrateTexture::height,const_cast<uint16_t*>(CrateTexture::pixels)}};
// Allocated once at startup and retained for the lifetime of this scene.
inline uint16_t* internalTexture=nullptr;
inline Renderer::Material material;
inline Renderer::Object* box=nullptr;
inline uint16_t background[Display::RENDER_HEIGHT];
inline float time=0;
inline int activeMode=-1;
inline void selectMode(int mode) {
    if(activeMode==mode) return;
    activeMode=mode;
    material.diffuseMap=&textures[mode];
    material.perspectiveCorrect=(mode&1)!=0;
    Labels::select(mode);
    constexpr const char* names[]={"AFFINE / NEAREST","PERSPECTIVE / NEAREST",
        "AFFINE / BILINEAR","PERSPECTIVE / BILINEAR"};
    std::printf("Texture mode: %s\n",names[mode]);
}
inline void init(Renderer::Scene& scene) {
    camera.setFOV(60.0f,Display::RENDER_WIDTH); camera.nearPlane=32; camera.farPlane=2000;
    scene.setCamera(&camera); scene.setDirectionalLight(&light); scene.setAmbientLight(&ambient);
    scene.setClearBuffer(true);
    for(int y=0;y<Display::RENDER_HEIGHT;++y) {
        const int red=12+y*24/Display::RENDER_HEIGHT;
        const int green=28+y*56/Display::RENDER_HEIGHT;
        const int blue=54+y*72/Display::RENDER_HEIGHT;
        background[y]=uint16_t((red>>3)<<11|(green>>2)<<5|(blue>>3));
    }
    scene.backgroundGradientColors=background;
    // Copy the shared 32 KiB texture into internal RAM to avoid flash-cache misses.
    // Keep the flash source as a fallback if a board has insufficient internal RAM.
#if defined(ESP_PLATFORM)
    if (!internalTexture) {
        internalTexture=static_cast<uint16_t*>(heap_caps_aligned_alloc(
            16,sizeof(CrateTexture::pixels),MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
        if (internalTexture)
            std::memcpy(internalTexture,CrateTexture::pixels,sizeof(CrateTexture::pixels));
    }
    std::printf("Crate texture: %dx%d, %u bytes, %s\n",CrateTexture::width,
        CrateTexture::height,unsigned(sizeof(CrateTexture::pixels)),
        internalTexture ? "internal DRAM" : "flash fallback");
#endif
    for(int i=0;i<4;++i) {
        if (internalTexture) textures[i].data=internalTexture;
        textures[i].addressMode=Renderer::CLAMP;
        textures[i].bilinear=i>=2;
    }
    material.diffuse=200;
    material.shadingMode=Renderer::ShadingMode::FLAT;
    box=Primitives::createCube(200,200,200,&material);
    box->setPosition(0,0,520);
    box->setRotation(22,35,0);
    box->cachePositions();
    scene.addObject(box);
    Labels::add(scene);
    selectMode(0);
}
inline void update(float seconds) {
    time=std::fmod(time+seconds,3600.0f);
    selectMode(int(time/3.0f)%4);
    const int pitch=int(22+18*std::sin(time*1.8f));
    const int yaw=int(std::fmod(35+time*65,360.0f));
    const int roll=int(8*std::sin(time*1.3f));
    box->setRotation(pitch,yaw,roll);
}
}
