#include "../vs_common.h"
#include "../World/instancing.h"

layout(location = 0) in vec3 POSITION;

out vec3 Normal;

void main() {
	mat4 view = ViewMatrix;
	view[3].xyz = vec3(0.0);
	gl_Position = ProjectionMatrix * view * vec4(POSITION * 1.0, 1.0);
	Normal = POSITION - vec3(0.0);
}