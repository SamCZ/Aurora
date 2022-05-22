layout(early_fragment_tests) in;

layout(location = 0) out vec4 FragColor;

in vec3 ViewVec;

uniform samplerCube CubeMap;

void main()
{
	FragColor = texture(CubeMap, normalize(ViewVec));
}