out vec4 FragColor;

in float Val;

void main()
{
	float AO = 1.0 - Val;

	FragColor = vec4(vec3(AO), 1.0);
}