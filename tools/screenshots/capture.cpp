#include DEMO_HEADER
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>
namespace Demo = DEMO_NAMESPACE;
static_assert(HALF_WIDTH_BUFFERS == 1 && FIELD_BUFFERS == 1,
              "These captures reconstruct the S3 half-width field output.");
int main(int argc, char **argv) {
    if (argc != 3) return 2;
    constexpr int w=480,h=320,n=w*h/4,guard=64;
    const int last=std::max(2,int(std::round(std::atof(argv[1])*60)));
    std::srand(123);
    std::vector<uint16_t> fields[2]={std::vector<uint16_t>(n+guard,0),std::vector<uint16_t>(n+guard,0)};
    for(auto& f:fields)std::fill(f.begin()+n,f.end(),0xA55A);
#if Z_BUFFERING
    std::vector<uint16_t> depth((w/2)*h+guard,65535);
    uint16_t* z=depth.data();
#else
    uint16_t* z=nullptr;
#endif
    Renderer::Scene scene(fields[0].data(),z,w,h);
    scene.getRenderer()->interlacedMode=true;
    Demo::init(scene);
    // The runtime normally attaches PerformanceOverlay. Deliberately omit it;
    // preserve all scene-authored captions, sprites and post effects.
    for(int frame=0;frame<=last;++frame){
        if(frame)Demo::update(1.f/60);
        // Simulate the complete timeline for particles/scene transitions, but
        // only rasterize the final second needed to settle field reflections.
        if(frame<last-60)continue;
        scene.frameCounter=frame;
        scene.setFramebuffer(fields[frame%2].data());
        scene.getRenderer()->reflectBuffer=fields[1-frame%2].data();
        scene.render();
#ifdef DEMO_EFFECTS
        Demo::DEMO_EFFECTS(scene);
#endif
#ifdef DEMO_AFTER
        Demo::DEMO_AFTER(1.f/60);
#endif
        for(auto& f:fields)for(int i=n;i<n+guard;++i)assert(f[i]==0xA55A);
#if Z_BUFFERING
        for(size_t i=(w/2)*h;i<depth.size();++i)assert(depth[i]==65535);
#endif
    }
    // A still screenshot pairs both physical field parities at the selected
    // pose. Keep S3 resolution/colour/shading, without motion combing from
    // combining two different animation instants into one still image.
    for(int field=0;field<4;++field){
        scene.frameCounter=field;
        scene.setFramebuffer(fields[field%2].data());
        scene.getRenderer()->reflectBuffer=fields[1-field%2].data();
        scene.render();
#ifdef DEMO_EFFECTS
        Demo::DEMO_EFFECTS(scene);
#endif
    }
#ifdef DEMO_S3_MEMORY_LABEL
    // sizeof(Triangle) contains a pointer and differs on a 64-bit desktop.
    // Show the checked S3 mesh-payload accounting, not the host ABI's size.
    auto s3Bytes=[](const Renderer::Object& o){
        return o.vertices.capacity()*36 + o.triangles.capacity()*16 + o.indices.capacity()*4
            + (o.cachedPositions()?o.vertices.size()*12:0)
            + (o.cachedPositionSources()?o.vertices.size()*2:0);
    };
    size_t bytes=0;
    if(Demo::prototype)bytes=s3Bytes(*Demo::prototype);
    else for(auto& o:Demo::objects)bytes+=s3Bytes(*o);
    Labels::select(Demo::activeMode,Demo::count,bytes);
#endif
    std::vector<uint16_t> full(w*h);
    for(int y=0;y<h;++y)for(int x=0;x<w;++x)
        full[y*w+x]=fields[(y&1)?0:1][(y/2)*(w/2)+x/2];
    auto sprites=scene.getSprites();
    std::stable_sort(sprites.begin(),sprites.end(),[](auto* a,auto* b){return a->zOrder<b->zOrder;});
    for(int y=0;y<h;++y)Renderer::compositeSprites(full.data()+y*w,w,y,sprites.data(),int(sprites.size()));
    std::ofstream output(argv[2],std::ios::binary);
    output<<"P6\n480 320\n255\n";
    for(auto p:full){char rgb[]={char(((p>>11)&31)*255/31),char(((p>>5)&63)*255/63),char((p&31)*255/31)};output.write(rgb,3);}
    if(!output)return 1;
    std::printf("Captured %s at %.3f seconds; %d rasterized triangles; no performance overlay\n",argv[2],last/60.f,scene.lastFrameRasterizedTriangles);
}
