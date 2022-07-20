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

uniform vec3 LightDir;

void main()
{
	vec3 N = normalize(Normal);

	NormalColor = vec4(N * 0.5f + 0.5f, 0.0f);
	FragColor = u_Color;

	float nDotL = dot(N, LightDir);
	nDotL = max(nDotL, 0.5);

	FragColor.rgb *= nDotL;
}