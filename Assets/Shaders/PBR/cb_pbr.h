#pragma once

#include "../ps_common.h"

uniformbuffer PBRDesc
{
	mat4 u_InvProjectionView;
	mat4 u_DirLightProjectionViewMatrix;
	vec4 CameraPos;
	vec4 TestOptions;
};

uniformbuffer DirectionalLight
{
	vec3 Direction;
#if defined(SHADER_ENGINE_SIDE)
	float Padding0;
#endif
	vec3 Radiance;
	float Multiplier;
} g_DirLight;