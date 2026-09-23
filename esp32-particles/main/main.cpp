#include "Runtime.hpp"
#include "ParticleDemo.hpp"
extern "C" void app_main() { Esp32Jet::start(ParticleDemo::init,ParticleDemo::update,nullptr,ParticleDemo::renderEffects); }
