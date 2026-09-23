#include "Runtime.hpp"
#include "CelTeapot.hpp"
extern "C" void app_main() { Esp32Jet::start(CelTeapot::init,CelTeapot::update); }
