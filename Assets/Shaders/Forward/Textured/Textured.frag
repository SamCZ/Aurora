layout(early_fragment_tests) in;

layout(location = 0) out vec4 FragColor;

#include "../../Decals.h"

uniform Color
{
	vec4 u_Tint;
};

in vec2 TexCoord;
in vec3 Normal;
in mat3 TBN;
in vec4 WorldPos;

#ifdef HAS_DECALS
in vec4 DecalProjections[10];
#endif

uniform sampler2D Texture;
#if USE_NORMAL_MAP
uniform sampler2D NormalMap;
#endif

#if USE_AO_MAP
uniform sampler2D AOMap;
#endif

uniform sampler2D g_DecalTexture;
uniform vec3 LightDir;

void main()
{
	FragColor = texture(Texture, TexCoord) * u_Tint;
#ifdef USE_ALPHA_THRESHOLD
	if(FragColor.a < 0.5)
		discard;
#endif

#if USE_AO_MAP
	FragColor.rgb *= texture(AOMap, TexCoord).rgb;
#endif

#if USE_NORMAL_MAP
	vec4 normalColor = texture(NormalMap, TexCoord);
	vec3 N = normalize(TBN * (normalColor.xyz * 2.0f - 1.0f));
#else
	vec3 N = normalize(Normal);
#endif

	float nDotL = dot(N, LightDir);
	nDotL = max(nDotL, 0.5);

	float d = dot(LightDir, vec3(0, 1, 0));
	d = clamp(d + 0.8f, 0.05f, 1.0f);

	FragColor.rgb *= nDotL * d;

#ifdef HAS_DECALS
	vec3 projCoords;
	vec4 decalColor;
	for (uint i = 0; i < DecalCountPS; i++)
	{
		projCoords = DecalProjections[i].xyz / DecalProjections[i].w;
		projCoords.xy = projCoords.xy * 0.5 + 0.5;

		if (projCoords[0] > 1.0 || projCoords[0] < 0.0
		|| projCoords[1] > 1.0 || projCoords[1] < 0.0
		|| projCoords[2] > 1.0 || projCoords[2] < 0.0)
			continue;

		decalColor = texture(g_DecalTexture, projCoords.xy);
		FragColor.rgb = mix(FragColor.rgb, decalColor.rgb, decalColor.a);
	}
#endif
}