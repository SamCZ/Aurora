#pragma once

#include "../ps_common.h"

uniformbuffer PBRDesc
{
	mat4 u_InvProjectionView;
	mat4 u_ViewMatrix;
	vec4 CameraPos;
	vec4 TestOptions;
};

uniformbuffer CascadeDesc
{
	mat4 u_CascadeMatrices[5];
	vec2 u_CascadeDistances[4];
};

uniformbuffer DirectionalLight
{
	vec3 Direction;
#if defined(SHADER_ENGINE_SIDE)
	float Padding0;
#endif
	vec3 Radiance;
	float Multiplier;
	vec2 ShadowIntensity;
} g_DirLight;