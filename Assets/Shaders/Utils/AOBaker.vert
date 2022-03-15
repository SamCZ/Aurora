layout(location = 0) in vec2 in_Pos;
layout(location = 1) in float in_Val;

uniform Matrices
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
};

out float Val;

void main()
{
	gl_Position = ProjectionMatrix * vec4(in_Pos.x, in_Pos.y, 0, 1);

	//Val = distance(gl_Position.xy, vec2(0, 0)) * in_Val;

	Val = in_Val;
}