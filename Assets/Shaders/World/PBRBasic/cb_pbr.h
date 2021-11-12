#pragma once
#include "../../common.h"

uniformbuffer PBRConstants
{
	vec4 BaseColor;
	float Roughness;
	float Metallic;
	float AmbientOcclusion;
	float _Padding0;
} pbrConstants;