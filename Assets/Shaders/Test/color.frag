out vec4 FragColor;

uniform Color
{
	vec4 u_Color;
};

in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D Texture;

void main()
{
	vec3 N = normalize(Normal);
	FragColor = u_Color;
	FragColor.rgb *= max(dot(N, vec3(0.5, 0.5, 0.5)), 0.2);
}