#pragma once

#if defined(SHADER_ENGINE_SIDE)
#include "Aurora/Core/Vector.hpp"

using namespace Aurora;

#define uniformbuffer struct
#define layout(...)

typedef Matrix4 mat4;
typedef Vector4 vec4;
typedef Vector2 vec2;
#endif

#if !defined(SHADER_ENGINE_SIDE)
#define uniformbuffer layout(std140) uniform
#endif