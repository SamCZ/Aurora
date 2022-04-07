layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec3 NormalColor;

uniform Color
{
	vec4 u_Tint;
};

in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D Texture;

void main()
{
	FragColor = texture(Texture, TexCoord) * u_Tint;

	if(FragColor.a < 0.5)
		discard;

	NormalColor = normalize(Normal) * 0.5f + 0.5f;
}