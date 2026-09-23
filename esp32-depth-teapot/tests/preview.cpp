#include "DepthTeapot.hpp"
#include "PerformanceOverlay.hpp"
#include <cassert>
#include <algorithm>
#include <thread>
#include <vector>

int main() {
    static_assert(Z_BUFFERING && JET_RUNTIME_DEPTH && !FAST_Z);
    constexpr int w=480,h=320,stride=w/2,count=stride*h/2,depthCount=stride*h;
    std::vector<uint16_t> pixels(count+16,0xbeef),depth(depthCount+16,0xd00d);
    Renderer::Scene scene(pixels.data(),depth.data(),w,h);
    scene.getRenderer()->interlacedMode=true;
    assert(scene.getRenderer()->isDepthTestingEnabled());
    DepthTeapot::init(scene);
    assert(!scene.getRenderer()->isDepthTestingEnabled());
    auto render=[&](int parity) {
        scene.frameCounter=parity;scene.prepareFrame();scene.rasterizeBand(0,h);
        assert(scene.lastFrameRasterizedTriangles>100);
        assert(std::all_of(pixels.begin()+count,pixels.end(),[](auto v){return v==0xbeef;}));
        assert(std::all_of(depth.begin()+depthCount,depth.end(),[](auto v){return v==0xd00d;}));
    };
    std::vector<uint16_t> montage(w*h*4);
    unsigned different=0,comparisons=0;
    for(int pose=0;pose<120;++pose) {
        const float phase=pose*(7.0f/120.0f);
        std::vector<uint16_t> painter[2];
        for(int mode=0;mode<2;++mode) {
            DepthTeapot::time=phase+mode*7.0f;DepthTeapot::update(0);
            assert(DepthTeapot::activeMode==mode);
            assert(DepthTeapot::glaze.shadingMode==Renderer::ShadingMode::PHONG);
            assert(scene.getRenderer()->isDepthTestingEnabled()==bool(mode));
            for(int parity=0;parity<2;++parity) {
                std::fill(depth.begin(),depth.end(),0xd00d);
                render(parity);
                const auto reference=pixels,referenceDepth=depth;
                if(mode==0) {
                    assert(std::all_of(depth.begin(),depth.end(),[](auto v){return v==0xd00d;}));
                    painter[parity]=pixels;
                } else {
                    assert(std::any_of(depth.begin(),depth.begin()+depthCount,[](auto v){return v!=0xd00d;}));
                    for(int i=0;i<count;++i)different+=painter[parity][i]!=pixels[i];
                }
                // Toggle and exercise odd band boundaries. Each worker inherits
                // the selected kernel without sharing mutable raster state.
                if(pose%15==0) for(int split:{1,159,200,319}) {
                    scene.prepareFrame();
                    std::vector<uint8_t> a(scene.lastFrameDrawnTriangles),b(a.size());
                    std::thread worker([&]{scene.rasterizeBand(split,h,b.data());});
                    scene.rasterizeBand(0,split,a.data());worker.join();
                    assert(pixels==reference && depth==referenceDepth);
                    ++comparisons;
                }
                if(pose==35||pose==45) {
                    const int row=pose==35?0:1;
                    for(int y=1-parity;y<h;y+=2) for(int x=0;x<w;++x)
                        montage[(row*h+y)*w*2+mode*w+x]=reference[(y/2)*stride+x/2];
                }
            }
        }
    }
    assert(different>1000); // Deliberately exposes the spout/body ordering error.
    // A disabled kernel must work without any depth storage at all.
    DepthTeapot::selectMode(0);scene.resize(pixels.data(),nullptr,w,h);render(0);
    scene.resize(pixels.data(),depth.data(),w,h);DepthTeapot::selectMode(1);render(1);
    for(float t:{0.f,6.999f,7.f,13.999f,14.f,20.999f,21.f}) {
        DepthTeapot::time=t;DepthTeapot::update(0);
        assert(DepthTeapot::activeMode==int(t/7)%2);
    }
    DepthTeapot::time=3583.99f;DepthTeapot::update(0.02f);
    assert(DepthTeapot::activeMode==0);
    PerformanceOverlay overlay;overlay.attach(scene,w);
    for(int row=0;row<2;++row)for(int mode=0;mode<2;++mode) {
        Labels::select(mode);
        const auto& sprites=scene.getSprites();
        for(int y=0;y<h;++y)Renderer::compositeSprites(montage.data()+(row*h+y)*w*2+mode*w,w,y,sprites.data(),int(sprites.size()));
    }
    FILE* f=std::fopen("depth-teapot.ppm","wb");assert(f);
    std::fprintf(f,"P6\n%d %d\n255\n",w*2,h*2);
    for(auto p:montage) {
        const unsigned char rgb[]={static_cast<unsigned char>(((p>>11)&31)*255/31),
            static_cast<unsigned char>(((p>>5)&63)*255/63),static_cast<unsigned char>((p&31)*255/31)};
        std::fwrite(rgb,1,3,f);
    }
    std::fclose(f);
    std::printf("Depth cycle: 120 matched poses, both parities, %u parallel comparisons, %u changed samples; null/stale depth and guards pass\n",comparisons,different);
}
