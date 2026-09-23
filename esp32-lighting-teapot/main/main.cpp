#include "Runtime.hpp"
#include "Teapot.hpp"

extern "C" void app_main() {
    Esp32Jet::start(Teapot::init, Teapot::update);
}
