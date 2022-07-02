#include "../../vs_common.h"

layout(location = 0) in vec4 in_Pos;

void main()
{
	gl_Position = ProjectionMatrix * (ViewMatrix * in_Pos);
}