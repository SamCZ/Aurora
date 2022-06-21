//layout(early_fragment_tests) in;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 NormalColor;

in vec2 TexCoord;
in vec3 Normal;
in mat3 TBN;
in vec4 WorldPos;

uniform vec3 LightDir;

void main()
{
	NormalColor = vec4(normalize(Normal) * 0.5f + 0.5f, 1.0);

	float nDotL = dot(normalize(Normal), LightDir);
	nDotL = max(nDotL, 0.5);

	FragColor = vec4(vec3(nDotL), 1.0);
}