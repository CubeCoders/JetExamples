#include "Frame.hpp"
#include <cstdlib>
int main(int argc, char **argv) {
    Frame f;
    if (argc > 1) {
        f.render(float(std::atof(argv[1])));
        Frame::save(argc > 2 ? argv[2] : "frame.ppm", f.out);
        return 0;
    }
    std::vector<uint16_t> montage(480 * 320 * 10);
    for (int i = 0; i < 10; ++i) {
        f.render(i * 18 + 7);
        for (int y = 0; y < 320; ++y)
            std::copy_n(f.out.data() + y * 480, 480,
                        montage.data() + ((i / 5) * 320 + y) * 2400 + (i % 5) * 480);
        std::printf("%02d: %d accepted\n", i + 1, f.target.lastFrameRasterizedTriangles);
    }
    Frame::save("matter.ppm", montage, 2400, 640);
}
