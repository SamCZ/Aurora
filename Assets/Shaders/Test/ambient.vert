#include "../vs_common.h"

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec2 TEXCOORD;
layout(location = 2) in vec3 NORMAL;

out vec2 TexCoord;
out vec3 Normal;

void main() {
	gl_Position = ProjectionMatrix * ViewMatrix * ModelTransform * vec4(POSITION, 1.0);
	TexCoord = TEXCOORD;
	Normal = mat3(ModelTransform) * NORMAL;
}