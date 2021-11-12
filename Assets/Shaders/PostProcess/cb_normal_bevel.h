#include "../common.h"

uniformbuffer NormalBevelEdgeData
{
	vec4 EdgeData;

#if defined(SHADER_ENGINE_SIDE)
	void Init(float edgeOffset, float edgeDistance, float bevelRadius, float bevelDistance)
	{
		EdgeData.x = edgeOffset;
		EdgeData.y = edgeDistance;
		EdgeData.z = bevelRadius;
		EdgeData.w = bevelDistance;
	}
#endif
};

#define EdgeOffset EdgeData.x
#define EdgeDistance EdgeData.y
#define BevelRadius EdgeData.z
#define BevelDistance EdgeData.w