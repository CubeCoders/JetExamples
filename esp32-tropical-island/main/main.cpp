#include "Runtime.hpp"
#include "Island.hpp"
extern "C" void app_main() { Esp32Jet::start(Island::init,Island::update,Island::afterRender); }
