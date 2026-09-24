#pragma once
#include "../../main/firmware/JetConfig.hpp"
// Exercise capabilities not needed by the visible, untextured sample.
#undef SORT_TRIANGLES
#define SORT_TRIANGLES 1
#undef TEXTURE_MAPPING
#define TEXTURE_MAPPING 1
#undef PERSPECTIVE_CORRECT_TEXTURES
#define PERSPECTIVE_CORRECT_TEXTURES 1
#undef BILINEAR_FILTER
#define BILINEAR_FILTER 1
#undef MAX_PICK_QUERIES
#define MAX_PICK_QUERIES 4
