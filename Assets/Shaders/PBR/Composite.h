#include "../ps_common.h"

#define MAX_DIRECTIONAL_LIGHTS 3
#define MAX_POINT_LIGHTS 1000

uniformbuffer CompositeDefaults
{
	mat4 InvProjectionView;
	mat4 ViewMatrix;
	vec4 _CameraPos;
	vec4 TestOptions;
};

struct DirectionalLightGPU
{
	vec4 DirectionIntensity;
	vec4 Color;
};

struct PointLightGPU
{
	vec4 PositionIntensity;
	vec4 ColorRadius;
};

uniformbuffer SkyLightStorage
{
	vec4 AmbientColorAndIntensity;
};

uniformbuffer DirectionalLightStorage
{
	DirectionalLightGPU DirLights[MAX_DIRECTIONAL_LIGHTS];
	uint DirLightCount;
};

uniformbuffer PointLightStorage
{
	PointLightGPU PointLights[MAX_POINT_LIGHTS];
	uint PointLightCount;
};