out vec2 TexCoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

const vec4 TrianglePos[4] =
{
	vec4(-1, -1, 0, 1),
	vec4(-1, 1, 0, 1),
	vec4(1, -1, 0, 1),
	vec4(1, 1, 0, 1),
};

const vec2 UVS[4] =
{
	vec2(0, 0),
	vec2(0, 1),
	vec2(1, 0),
	vec2(1, 1),
};

void main()
{
	gl_Position = TrianglePos[gl_VertexID];
	TexCoord = UVS[gl_VertexID];
}