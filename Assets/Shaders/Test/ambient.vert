#include "../vs_common.h"
#include "../World/instancing.h"

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec2 TEXCOORD;
layout(location = 2) in vec3 NORMAL;
layout(location = 3) in vec3 TANGENT;
layout(location = 4) in vec3 BITANGENT;

out vec2 TexCoord;
out vec3 Normal;
out mat3 TBN;

void main() {
	gl_Position = ProjectionMatrix * ViewMatrix * INST_TRANSFORM * vec4(POSITION, 1.0);
	TexCoord = TEXCOORD;
	Normal = mat3(INST_TRANSFORM) * NORMAL;

	vec3 T = normalize((INST_TRANSFORM * vec4(TANGENT, 0.0)).xyz);
	vec3 B = normalize((INST_TRANSFORM * vec4(BITANGENT, 0.0)).xyz);
	vec3 N = normalize((INST_TRANSFORM * vec4(NORMAL, 0.0)).xyz);

	TBN = mat3(T, B, N);
}