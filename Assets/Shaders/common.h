#pragma once

#ifdef SHADER_ENGINE_SIDE
#include "Aurora/Core/Vector.hpp"

using namespace Aurora;

#define uniformbuffer struct
#define layout(...)

typedef Matrix4 mat4;
#else
#define uniformbuffer uniform
#endif