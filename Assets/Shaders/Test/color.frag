layout(early_fragment_tests) in;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 NormalColor;

uniform Color
{
	vec4 u_Color;
};

in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D Texture;

void main()
{
	NormalColor = vec4(normalize(Normal) * 0.5f + 0.5f, 0.0f);
	FragColor = u_Color;
}