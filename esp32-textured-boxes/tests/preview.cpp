#include "Boxes.hpp"
#include "PerformanceOverlay.hpp"
#include <cassert>
#include <vector>
#include <algorithm>

int main() {
    constexpr int w=480, h=320, stride=w/2, count=stride*h/2;
    std::vector<uint16_t> fields[2] = {std::vector<uint16_t>(count+16),std::vector<uint16_t>(count+16)};
    std::vector<uint16_t> depth(stride*h);
    Renderer::Scene scene(fields[0].data(),depth.data(),w,h);
    scene.getRenderer()->interlacedMode = true;
    Boxes::init(scene);
    PerformanceOverlay display;
    display.attach(scene,w);
    PerformanceOverlay stats;
    stats.tick(1000,0);
    for (int frame=1;frame<=30;++frame) stats.tick(1000+frame*20000,800);
    assert(stats.fpsTenths()==500 && stats.triangles()==800 && stats.trianglesPerSecond()==40000);
    PerformanceOverlay varying;
    varying.tick(1000,0);
    for(int frame=1;frame<=10;++frame) varying.tick(1000+frame*50000,frame*200);
    assert(varying.fpsTenths()==200 && varying.triangles()==2000);
    assert(varying.trianglesPerSecond()==22000); // Sum of actual fields, not FPS * latest count.
    assert(Boxes::box->triangles.size()==12);
    for(int i=0;i<4;++i) assert(Boxes::textures[i].bilinear==(i>=2));
    std::vector<uint16_t> montage(4*w*h), lit;
    auto render = [&]() {
        for(int field=0;field<2;++field) {
            auto& buffer=fields[field];
            std::fill(buffer.begin()+count,buffer.end(),0xbeef);
            scene.setFramebuffer(buffer.data());
            scene.render();
            assert(scene.lastFrameDrawnObjects==1 && scene.lastFrameRasterizedTriangles>=2);
            assert(std::all_of(buffer.begin()+count,buffer.end(),[](auto p){return p==0xbeef;}));
        }
        std::vector<uint16_t> pixels(w*h);
        int visible=0;
        for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
            auto p=fields[(y&1)?0:1][(y/2)*stride+x/2];
            pixels[y*w+x]=p;
            visible+=p!=Boxes::background[y];
        }
        assert(visible>3000 && visible<60000);
        return pixels;
    };
    for(int pose=0;pose<4;++pose) {
        Boxes::time=0;
        Boxes::update(0);
        Boxes::selectMode(pose);
        auto pixels=render();
        auto& sprites=scene.getSprites();
        for(int y=0;y<h;++y) {
            Renderer::compositeSprites(pixels.data()+y*w,w,y,sprites.data(),int(sprites.size()));
            std::copy_n(pixels.data()+y*w,w,montage.data()+((pose/2)*h+y)*w*2+(pose%2)*w);
        }
    }
    for(float t:{0.0f,2.999f,3.0f,5.999f,6.0f,8.999f,9.0f,11.999f,12.0f}) {
        Boxes::time=t;Boxes::update(0);
        const int mode=int(t/3)%4;
        assert(Boxes::activeMode==mode && Boxes::material.perspectiveCorrect==bool(mode&1));
        assert(Boxes::material.diffuseMap->bilinear==(mode>=2));
    }
    Boxes::time=3599.99f;Boxes::update(0.02f);assert(Boxes::activeMode==0);
    FILE* f=std::fopen("boxes.ppm","wb"); assert(f);
    std::fprintf(f,"P6\n%d %d\n255\n",w*2,h*2);
    for(auto p:montage) {
        const unsigned char rgb[]={static_cast<unsigned char>(((p>>11)&31)*255/31),
            static_cast<unsigned char>(((p>>5)&63)*255/63),static_cast<unsigned char>((p&31)*255/31)};
        std::fwrite(rgb,1,3,f);
    }
    std::fclose(f);
    std::puts("Box: four variants at the same pose, cycle timing and field guards pass");
}
