#pragma once
#include "Scene.hpp"
#include <array>
#include <algorithm>
#include <cstdio>

// The frame task updates these pixels only between completed frames, before
// notifying the render task. Texture storage is immutable throughout scanout.
class PerformanceOverlay {
    static constexpr int width=108, height=38;
    std::array<uint16_t,width*height> pixels{};
    Renderer::Texture texture{width,height,pixels.data()};
    Renderer::Material material{0xffff,&texture};
    Renderer::Sprite2D sprite;
    int64_t sampleStart=0;
    unsigned intervals=0;
    uint64_t triangleSum=0;
    unsigned fps10=0, currentTriangles=0, triangleRate=0;
    static const uint8_t* glyph(char c) {
        static constexpr uint8_t data[][5]={
            {0x3e,0x51,0x49,0x45,0x3e}, {0,0x42,0x7f,0x40,0},
            {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4b,0x31},
            {0x18,0x14,0x12,0x7f,0x10}, {0x27,0x45,0x45,0x45,0x39},
            {0x3c,0x4a,0x49,0x49,0x30}, {1,0x71,9,5,3},
            {0x36,0x49,0x49,0x49,0x36}, {6,0x49,0x49,0x29,0x1e},
            {0x7f,9,9,9,1}, {0x7f,9,9,9,6}, {0x46,0x49,0x49,0x49,0x31},
            {0,0x60,0x60,0,0}, {8,8,8,8,8}, {0,0,0,0,0},
            {1,1,0x7f,1,1}, {0x7f,9,0x19,0x29,0x46},
            {0,0x41,0x7f,0x41,0}, {0x20,0x10,8,4,2}};
        if(c>='0' && c<='9') return data[c-'0'];
        return data[c=='F'?10:c=='P'?11:c=='S'?12:c=='.'?13:c=='-'?14:c=='T'?16:c=='R'?17:c=='I'?18:c=='/'?19:15];
    }
    void line(const char* text, int top, int scale) {
        for(int ch=0;text[ch] && (ch+1)*6*scale<=width;++ch) for(int x=0;x<5;++x)
            for(int y=0;y<7;++y) if(glyph(text[ch])[x]&(1<<y))
                for(int yy=0;yy<scale;++yy) for(int xx=0;xx<scale;++xx)
                    pixels[(top+y*scale+yy)*width+ch*6*scale+x*scale+xx]=0xffff;
    }
    void draw(bool ready) {
        pixels.fill(0x0843);
        char label[32];
        if (ready) std::snprintf(label,sizeof(label),"FPS %2u.%u",fps10/10,fps10%10);
        else std::snprintf(label,sizeof(label),"FPS --.-");
        line(label,0,2);
        std::snprintf(label,sizeof(label),"TRIS  %u",currentTriangles);
        line(label,18,1);
        std::snprintf(label,sizeof(label),"TRI/S %u",triangleRate);
        line(label,29,1);
    }
public:
    void attach(Renderer::Scene& scene,int screenWidth) {
        sprite.x=screenWidth-width-16; sprite.y=17;
        sprite.material=&material; sprite.zOrder=1000000;
        draw(false);
        scene.addSprite(&sprite);
    }
    // Call between completed frames. Count unique rasterized triangles after
    // culling (the two raster bands have already merged their flags).
    void tick(int64_t now, unsigned completedTriangles) {
        if(!sampleStart) { sampleStart=now; return; }
        ++intervals;
        triangleSum += completedTriangles;
        currentTriangles = completedTriangles;
        if(now-sampleStart<500000) return;
        fps10=std::min(999u,unsigned(intervals*10000000ULL/(now-sampleStart)));
        triangleRate=unsigned(triangleSum*1000000ULL/(now-sampleStart));
        draw(true);
        intervals=0; triangleSum=0; sampleStart=now;
    }
    unsigned fpsTenths() const { return fps10; }
    unsigned triangles() const { return currentTriangles; }
    unsigned trianglesPerSecond() const { return triangleRate; }
};
