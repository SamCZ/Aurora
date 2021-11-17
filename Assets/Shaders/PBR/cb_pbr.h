#pragma once

#include "../ps_common.h"

uniformbuffer PBRDesc
{
	mat4 u_InvProjectionView;
	vec4 CameraPos;
	vec4 TestOptions;
};