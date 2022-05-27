#include "common.h"

#define MAX_DECALS_PER_OBJECT 10

uniformbuffer GLOB_DecalMatricesVS
{
	mat4 DecalMatrices[MAX_DECALS_PER_OBJECT];
	uint DecalCountVS;
};

uniformbuffer GLOB_DecalMatricesPS
{
	vec4 DecalOffsets[MAX_DECALS_PER_OBJECT];
	uint DecalCountPS;
};