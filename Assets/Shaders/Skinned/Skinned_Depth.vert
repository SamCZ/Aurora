#include "../vs_common.h"
#include "../World/instancing.h"

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec2 TEXCOORD;

layout(location = 5) in ivec4 BONEINDICES;
layout(location = 6) in vec4 BONEWEIGHTS;

out vec2 TexCoord;
out vec3 Normal;
out mat3 TBN;

#define MAX_BONES 120

uniform GLOB_BoneData
{
	mat4x4 g_Bones[MAX_BONES];
};

void main()
{
	mat4 boneTransform = g_Bones[BONEINDICES[0]] * BONEWEIGHTS[0];
	boneTransform += g_Bones[BONEINDICES[1]] * BONEWEIGHTS[1];
	boneTransform += g_Bones[BONEINDICES[2]] * BONEWEIGHTS[2];
	boneTransform += g_Bones[BONEINDICES[3]] * BONEWEIGHTS[3];

	vec4 worldPos = INST_TRANSFORM * boneTransform * vec4(POSITION, 1.0);
	gl_Position = ProjectionMatrix * (ViewMatrix * worldPos);
	TexCoord = TEXCOORD;
}