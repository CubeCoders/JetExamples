#include "../components/esp32_jet/FrameSchedule.hpp"
#include <cassert>
#include <cstdio>

int main() {
    FrameSchedule clock;
    clock.reset(0);
    int64_t previous = 0;
    for (int frame = 1; frame <= 60000; ++frame) {
        // Variable work and timer wake latency stay below the next deadline.
        const int64_t next = clock.nextFrame(previous + 80 + frame % 15000);
        assert(next == int64_t(frame) * 1000000 / 60);
        previous = next;
    }
    const int64_t overrun = previous + 100000;
    assert(clock.nextFrame(overrun) == overrun);
    const int64_t resumed = clock.nextFrame(overrun + 1000);
    assert(resumed - overrun >= 16666 && resumed - overrun <= 16667);
    // A long suspension also starts immediately, without queued catch-up frames.
    const int64_t late = resumed + 10000000;
    assert(clock.nextFrame(late) == late);
    assert(clock.nextFrame(late) > late);
    clock.reset(123456789);
    assert(clock.nextFrame(123456789) == 123473455);
    puts("Frame schedule: 60,000 deadlines, overruns and restart PASS");
}
