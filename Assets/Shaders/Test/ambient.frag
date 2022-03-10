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
	FragColor = texture(Texture, TexCoord);
	FragColor.rgb *= clamp(dot(N, vec3(0.5, 0.5, 0.5)), 0.05, 1.0);
}