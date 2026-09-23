#include "Runtime.hpp"
#include "DepthTeapot.hpp"
extern "C" void app_main() { Esp32Jet::start(DepthTeapot::init, DepthTeapot::update); }
