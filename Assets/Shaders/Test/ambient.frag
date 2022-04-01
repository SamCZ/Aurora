out vec4 FragColor;

uniform Color
{
	vec4 u_Tint;
};

in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D Texture;

void main()
{
	vec3 N = normalize(Normal);
	FragColor = texture(Texture, TexCoord) * u_Tint;

	if(FragColor.a < 0.5)
		discard;

	FragColor.rgb *= max(dot(N, vec3(0.5, 0.5, 0.5)), 0.2);
}