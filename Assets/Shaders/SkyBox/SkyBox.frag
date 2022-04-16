layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec3 NormalColor;
in vec3 Normal;

uniform samplerCube CubeMap;

void main()
{
	FragColor = texture(CubeMap, normalize(Normal));

	NormalColor = vec3(0.0);
}