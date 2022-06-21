#include "../vs_common.h"
#include "../World/instancing.h"

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec2 TEXCOORD;
layout(location = 2) in vec3 NORMAL;
layout(location = 3) in vec3 TANGENT;
layout(location = 4) in vec3 BITANGENT;

layout(location = 5) in ivec4 BONEINDICES;
layout(location = 6) in vec4 BONEWEIGHTS;

out vec2 TexCoord;
out vec3 Normal;
out mat3 TBN;
out vec4 WorldPos;

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

	WorldPos = INST_TRANSFORM * boneTransform * vec4(POSITION, 1.0);
	gl_Position = ProjectionMatrix * (ViewMatrix * WorldPos);
	TexCoord = TEXCOORD;
	Normal = mat3(INST_TRANSFORM) * NORMAL;

	vec3 T = normalize((INST_TRANSFORM * boneTransform * vec4(TANGENT, 0.0)).xyz);
	vec3 B = normalize((INST_TRANSFORM * boneTransform * vec4(BITANGENT, 0.0)).xyz);
	vec3 N = normalize((INST_TRANSFORM * boneTransform * vec4(NORMAL, 0.0)).xyz);

	TBN = mat3(T, B, N);
}