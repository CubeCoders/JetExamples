#include "Island.hpp"
#include "PerformanceOverlay.hpp"
#include <vector>
#include <thread>
#include <cassert>
#include <algorithm>
#include <cstdio>

int main() {
    constexpr int w=480,h=320,stride=w/2,count=stride*h/2;
    std::vector<uint16_t> fields[2]={std::vector<uint16_t>(count+16,0xbeef),std::vector<uint16_t>(count+16,0xbeef)};
    std::vector<uint16_t> depth(stride*h+16,0xffff);
    Renderer::Scene scene(fields[0].data(),depth.data(),w,h);
    scene.getRenderer()->interlacedMode=true;
    Island::init(scene);
    PerformanceOverlay stats;stats.attach(scene,w);
    int objs,tris,verts;scene.getStatistics(objs,tris,verts);
    assert(tris>400 && tris<1200);
    std::printf("Island: %d objects, %d triangles, %d vertices\n",objs,tris,verts);
    std::vector<uint16_t> montage(4*w*h);
    bool sawFlare=false,sawHiddenFlare=false;
    for(int pose=0;pose<4;++pose) {
        constexpr float times[]={0,3.75f,22.5f,26.25f}; Island::time=times[pose];
        for(int frame=0;frame<8;++frame) {
            int field=frame%2;
            scene.frameCounter=frame;
            scene.setFramebuffer(fields[field].data());
            scene.getRenderer()->reflectBuffer=fields[1-field].data();
            const auto previous=fields[1-field];
            Island::update(0);
            scene.render();
            Island::afterRender(1.f/30);
            assert(fields[1-field]==previous); // Immutable previous-field reflection source.
            assert(scene.lastFrameDrawnObjects>10 && scene.lastFrameRasterizedTriangles>40);
            assert(std::all_of(fields[field].begin()+count,fields[field].end(),[](auto p){return p==0xbeef;}));
            assert(std::all_of(depth.begin()+stride*h,depth.end(),[](auto p){return p==0xffff;}));
        }
        // Same immutable reflection source and prepared frame, across band splits.
        scene.frameCounter=8;scene.prepareFrame();scene.rasterizeBand(0,h);
        const auto reference=fields[1], referenceDepth=depth;
        const auto referencePick=scene.getPickResults()[0];
        const int referenceTriangles=scene.lastFrameRasterizedTriangles;
        for(int split:{31,160,199}) {
            scene.prepareFrame();
            std::vector<uint8_t> upper(scene.lastFrameDrawnTriangles),lower(upper.size());
            std::thread worker([&]{scene.rasterizeBand(split,h,lower.data());});
            scene.rasterizeBand(0,split,upper.data());worker.join();
            assert(fields[1]==reference && depth==referenceDepth);
            int drawn=0;for(size_t i=0;i<upper.size();++i) drawn+=(upper[i]|lower[i])!=0;
            assert(drawn==referenceTriangles);
            assert(scene.getPickResults()[0].hit==referencePick.hit);
        }
        bool active=false;
        for(auto* s:scene.getSprites()) if(s->blendMode==Renderer::BlendMode::BLEND_ADD && s->enabled && s->alpha) active=true;
        sawFlare|=active;sawHiddenFlare|=!active;
        auto& sprites=scene.getSprites();
        for(int y=0;y<h;++y) {
            auto* row=montage.data()+((pose/2)*h+y)*(w*2)+(pose%2)*w;
            for(int x=0;x<w;++x) row[x]=fields[(y&1)?0:1][(y/2)*stride+x/2];
            Renderer::compositeSprites(row,w,y,sprites.data(),int(sprites.size()));
        }
    }
    assert(sawFlare && sawHiddenFlare);
    Island::time=3599.99f;Island::update(.02f);assert(Island::time<.1f);
    FILE* f=std::fopen("island.ppm","wb");assert(f);
    std::fprintf(f,"P6\n%d %d\n255\n",w*2,h*2);
    for(auto p:montage) {
        const unsigned char rgb[]={static_cast<unsigned char>(((p>>11)&31)*255/31),
            static_cast<unsigned char>(((p>>5)&63)*255/63),static_cast<unsigned char>((p&31)*255/31)};
        std::fwrite(rgb,1,3,f);
    }
    std::fclose(f);
    std::puts("Island: orbit, flare visibility, immutable reflections, guards and parallel bands pass");
}
