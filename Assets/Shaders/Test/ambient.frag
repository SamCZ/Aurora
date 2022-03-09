out vec4 FragColor;

uniform Color
{
	vec4 u_Color;
};

in vec3 Normal;

void main()
{
	vec3 N = normalize(Normal);
	FragColor = vec4(1.0);
	FragColor.rgb *= clamp(dot(N, vec3(0.5, 0.5, 0.5)), 0.05, 1.0);
}