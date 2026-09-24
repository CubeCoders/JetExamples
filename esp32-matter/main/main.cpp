#include "Runtime.hpp"
#include "Exhibition.hpp"
extern "C" void app_main() {
    Esp32Jet::start(Matter::init, Matter::update, nullptr, Matter::effects);
}
