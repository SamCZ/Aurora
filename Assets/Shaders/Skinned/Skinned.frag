//layout(early_fragment_tests) in;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 NormalColor;

in vec2 TexCoord;
in vec3 Normal;
in mat3 TBN;
in vec4 WorldPos;

uniform vec3 LightDir;

uniform sampler2D BaseColor;
uniform sampler2D NormalMap;
uniform sampler2D AOMap;

void main()
{
	NormalColor = vec4(normalize(Normal) * 0.5f + 0.5f, 1.0);

	vec4 color = texture(BaseColor, TexCoord);

	vec3 normalColor = texture(NormalMap, TexCoord).rgb * 2.0f - 1.0f;
	vec3 N = normalize(TBN * normalColor);

	float nDotL = dot(N, LightDir);
	nDotL = max(nDotL, 0.5);

	float ao = texture(AOMap, TexCoord).r;

	FragColor = vec4(color.rgb * nDotL * ao, 1.0);
}