#include "../../vs_common.h"
#include "../../World/instancing.h"
#include "../../Decals.h"
#include "../../Shadows.h"

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec2 TEXCOORD;
layout(location = 2) in vec3 NORMAL;
layout(location = 3) in vec3 TANGENT;
layout(location = 4) in vec3 BITANGENT;

out vec2 TexCoord;
out vec3 Normal;
out mat3 TBN;
out vec4 WorldPos;

uniform highp mat4 ShadowmapMatrix[NUM_SHADOW_MAP_LEVELS];
out highp vec3 ShadowCoords[NUM_SHADOW_MAP_LEVELS];

#ifdef HAS_DECALS
out vec4 DecalProjections[10];
#endif

void main()
{
	WorldPos = INST_TRANSFORM * vec4(POSITION, 1.0);
	gl_Position = ProjectionMatrix * (ViewMatrix * WorldPos);
	TexCoord = TEXCOORD;
	Normal = mat3(INST_TRANSFORM) * NORMAL;

	for(int i = 0; i < ShadowmapMatrix.length(); i++)
	{
		ShadowCoords[i] = (ShadowmapMatrix[i] * WorldPos).xyz;
	}

#ifdef HAS_DECALS
	for (uint i = 0; i < DecalCountVS; i++)
	{
		DecalProjections[i] = DecalMatrices[i] * WorldPos;
	}
#endif
	vec3 T = normalize((INST_TRANSFORM * vec4(TANGENT, 0.0)).xyz);
	vec3 B = normalize((INST_TRANSFORM * vec4(BITANGENT, 0.0)).xyz);
	vec3 N = normalize((INST_TRANSFORM * vec4(NORMAL, 0.0)).xyz);

	TBN = mat3(T, B, N);
}