#include "Runtime.hpp"
#include "TextureLab.hpp"
extern "C" void app_main() { Esp32Jet::start(TextureLab::init,TextureLab::update); }
