#include "Teapot.hpp"
#include <thread>
#include <vector>
#include <cassert>
#include <algorithm>
#include <cstdio>

int main() {
    constexpr int width=480,height=320,stride=width/2,count=stride*height/2;
    std::vector<uint16_t> color(count+16,0xbeef),depth(stride*height+16,0xffff);
    Renderer::Scene scene(color.data(),depth.data(),width,height);
    scene.getRenderer()->interlacedMode=true;
    Teapot::init(scene);
    unsigned comparisons=0;
    for(int pose=0;pose<8;++pose) {
        Teapot::time=pose*1.8f; Teapot::update(0);
        scene.frameCounter=pose;
        scene.prepareFrame();
        scene.rasterizeBand(0,height);
        const auto reference=color,referenceDepth=depth;
        const int referenceTriangles=scene.lastFrameRasterizedTriangles;
        for(int split:{1,31,160,199,319}) {
            scene.prepareFrame();
            std::vector<uint8_t> upper(scene.lastFrameDrawnTriangles),lower(upper.size());
            std::thread worker([&]{scene.rasterizeBand(split,height,lower.data());});
            scene.rasterizeBand(0,split,upper.data());
            worker.join();
            assert(color==reference && depth==referenceDepth);
            int triangles=0;
            for(size_t i=0;i<upper.size();++i) triangles+=(upper[i]|lower[i])!=0;
            assert(triangles==referenceTriangles);
            ++comparisons;
        }
    }
    std::printf("Parallel depth: %u odd/even splits match serial colour, depth, guards and counts exactly\n",comparisons);
}
