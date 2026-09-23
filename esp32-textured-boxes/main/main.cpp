#include "Runtime.hpp"
#include "Boxes.hpp"
extern "C" void app_main() { Esp32Jet::start(Boxes::init,Boxes::update); }
