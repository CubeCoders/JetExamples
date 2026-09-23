#include "PerformanceOverlay.hpp"
#include <cassert>
int main() {
    // Identical rendering, different display pacing: TRI/S and MS must agree.
    PerformanceOverlay fast, slow;
    fast.tick(0,0,0); slow.tick(0,0,0);
    for (int i=1;i<=50;++i) fast.tick(i*10000,600,5000);
    for (int i=1;i<=10;++i) slow.tick(i*50000,600,5000);
    assert(fast.fpsTenths()==999 && slow.fpsTenths()==200);
    assert(fast.trianglesPerSecond()==120000 && slow.trianglesPerSecond()==120000);
    assert(fast.renderMillisecondsTenths()==50 && slow.renderMillisecondsTenths()==50);
    // Weighted aggregate, not mean of per-frame rates; next window resets.
    fast.tick(750000,100,10000); fast.tick(1000000,900,30000);
    assert(fast.trianglesPerSecond()==25000 && fast.renderMillisecondsTenths()==200);
    fast.tick(1500000,0,0);
    assert(fast.trianglesPerSecond()==0 && fast.renderMillisecondsTenths()==0);
}
