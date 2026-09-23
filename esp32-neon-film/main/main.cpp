#include "Runtime.hpp"
#include "Film.hpp"
extern "C" void app_main(){Esp32Jet::start(Film::init,Film::update,nullptr,Film::effects);}
