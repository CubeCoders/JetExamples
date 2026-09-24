#include "Instancing.hpp"
#include "Runtime.hpp"
extern "C" void app_main() { Esp32Jet::start(Instancing::init, Instancing::update); }
