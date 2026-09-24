#pragma once
#include "../../main/firmware/JetConfig.hpp"
#undef HALF_WIDTH_BUFFERS
#define HALF_WIDTH_BUFFERS 0
#undef FIELD_BUFFERS
#define FIELD_BUFFERS 0
#undef SSR_FIELD_REFLECT
#define SSR_FIELD_REFLECT 0
#undef Z_BUFFERING
#define Z_BUFFERING 1
#undef FAST_Z
#define FAST_Z 0
#undef BILINEAR_FILTER
#define BILINEAR_FILTER 1
#define JET_HIGH_PRECISION_UVS 1
#define JET_PERSPECTIVE_DEPTH 1
#define MATTER_RENDER_SCALE 6
