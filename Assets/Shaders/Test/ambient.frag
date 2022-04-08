layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec3 NormalColor;

uniform Color
{
	vec4 u_Tint;
};

in vec2 TexCoord;
in vec3 Normal;
in mat3 TBN;

uniform sampler2D Texture;
#if USE_NORMAL_MAP
uniform sampler2D NormalMap;
#endif

void main()
{
	FragColor = texture(Texture, TexCoord) * u_Tint;

	if(FragColor.a < 0.5)
		discard;

#if USE_NORMAL_MAP
	vec4 normalColor = texture(NormalMap, TexCoord);
	vec3 normalFromTex = normalize(TBN * (normalColor.xyz * 2.0f - 1.0f));
	NormalColor = normalFromTex * 0.5f + 0.5f;
#else
	NormalColor = normalize(Normal) * 0.5f + 0.5f;
#endif
}