#include "Runtime.hpp"
#include "Workshop.hpp"
extern "C" void app_main() { Esp32Jet::start(Workshop::init,Workshop::update); }
