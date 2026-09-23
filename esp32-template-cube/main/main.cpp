#include "Runtime.hpp"
#include "Cube.hpp"

extern "C" void app_main() {
    Esp32Jet::start(Cube::init, Cube::update);
}
