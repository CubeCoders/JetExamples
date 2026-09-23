#include "Runtime.hpp"
#include "CrtDemo.hpp"
extern "C" void app_main() { Esp32Jet::start(CrtDemo::init,CrtDemo::update); }
