#include "vs_common.h"
#include "World/instancing.h"

layout(location = 0) in vec3 POSITION;

void main() {
	gl_Position = ProjectionMatrix * ViewMatrix * INST_TRANSFORM * vec4(POSITION, 1.0);
}