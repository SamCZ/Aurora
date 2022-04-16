#include "../vs_common.h"
#include "../World/instancing.h"

layout(location = 0) in vec3 POSITION;
layout(location = 2) in vec3 NORMAL;

out vec3 Normal;

void main() {
	mat4 view = ViewMatrix;
	view[3] = vec4(0, 0, 0, 1);
	gl_Position = ProjectionMatrix * view * vec4(POSITION, 1.0);
	Normal = NORMAL;
}