layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec3 NormalColor;
in vec3 Normal;

void main()
{
	FragColor.rgb = vec3(1.0);
	FragColor.a = 1.0f;

	NormalColor = vec3(0.0);
}