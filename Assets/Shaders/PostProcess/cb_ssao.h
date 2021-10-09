#include "../ps_common.h"

#define SSAO_SAMPLE_COUNT 64

uniformbuffer SSAODesc
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
	vec4 NoiseData;
	vec4 Samples[SSAO_SAMPLE_COUNT];
};

#define NoiseScale NoiseData.xy
#define SSAORadius NoiseData.z
#define SSAOBias NoiseData.w