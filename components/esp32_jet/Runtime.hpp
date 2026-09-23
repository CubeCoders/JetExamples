#pragma once
#include "Scene.hpp"

namespace Esp32Jet {
// Call once from app_main. The runtime owns the Scene and framebuffers.
// init runs on core 0 after display setup; update runs on core 1 before each
// render, with actual elapsed seconds. Keep borrowed objects/camera alive.
// The callbacks must not access the panel or start their own render passes.
using Init = void (*)(Renderer::Scene&);
using Update = void (*)(float seconds);
void start(Init init, Update update);
}
