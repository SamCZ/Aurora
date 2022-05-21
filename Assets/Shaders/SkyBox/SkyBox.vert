#include "../vs_common.h"
#include "../World/instancing.h"

layout(location = 0) in vec3 POSITION;
layout(location = 2) in vec3 NORMAL;

out vec3 Normal;

void main() {
	#ifdef SKY_MAPPING
	mat4 view = ViewMatrix;
	view[3].xyz = vec3(0.0);
	gl_Position = ProjectionMatrix * view * vec4(POSITION * 10.0, 1.0);
	//gl_Position.z = 0;
	#else
	gl_Position = ProjectionMatrix * ViewMatrix * INST_TRANSFORM * vec4(POSITION, 1.0);
	#endif
	Normal = POSITION - vec3(0.0);
}