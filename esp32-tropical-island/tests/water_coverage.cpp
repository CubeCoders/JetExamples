#include "Island.hpp"
#include <vector>
#include <cassert>
#include <cstdio>
#include <algorithm>
int main() {
    constexpr int w=480,h=320,stride=w/2,count=stride*h/2;
    std::vector<uint16_t> color(count+16,0xbeef),depth(stride*h+16,0xffff);
    Renderer::Scene scene(color.data(),depth.data(),w,h);
    scene.getRenderer()->interlacedMode=true;
    Island::init(scene);
    Renderer::Material solid(0xf800);solid.shadingMode=Renderer::ShadingMode::UNLIT;
    for(auto* o:scene.getObjects()) {
        bool water=o->triangles[0].material->shadingMode==Renderer::ShadingMode::WATER_REFLECT;
        o->enabled=water;
        if(water) for(auto& t:o->triangles) t.material=&solid;
    }
    int worst=0;float worstTime=0;
    for(int pose=0;pose<240;++pose) {
        Island::time=pose*.125f;Island::update(0);
        for(int field=0;field<2;++field) {
            scene.frameCounter=field;scene.render();
            int holes=0;
            // Every ray here intersects the finite ocean well inside its extent.
            for(int y=230+(field?0:1);y<318;y+=2) for(int x=2;x<stride-2;++x)
                holes+=color[(y/2)*stride+x]!=0xf800;
            if(holes>worst) {worst=holes;worstTime=Island::time;}
        }
    }
    std::printf("Ocean coverage: worst missing pixels %d at time %.2f\n",worst,worstTime);
    std::fflush(stdout);
    assert(worst==0); // No missing wedges or shared-edge cracks.
}
