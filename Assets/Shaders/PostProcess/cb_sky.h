#include "../ps_common.h"

struct AtmosphereData
{
	vec4 data0;
	vec4 scatteringCoefficients;
};

uniformbuffer SkyConstants
{
	mat4 InvProjection;
	mat4 InvView;
	vec4 CameraPos;
	vec4 ViewPort;
	AtmosphereData atmosphereData;
};

#define planetRadius atmosphereData.data0.x
#define atmosphereRadius atmosphereData.data0.y
#define densityFallOff atmosphereData.data0.z