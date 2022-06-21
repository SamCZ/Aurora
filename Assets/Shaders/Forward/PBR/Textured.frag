layout(early_fragment_tests) in;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 NormalColor;

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

uniform sampler2D AO;

uniform sampler2D g_DecalTexture;

vec3 getTriPlanarBlend(vec3 normal)
{
	vec3 blending = abs( normal );
	blending = normalize(max(blending, 0.00001)); // Force weights to sum to 1.0
	float b = (blending.x + blending.y + blending.z);
	blending /= vec3(b, b, b);

	/*vec3 blend = vec3(0.0);
	// Blend for sides only
	vec2 xzBlend = abs(normalize(normal.xz));
	blend.xz = maxv(0, xzBlend - 0.67);
	blend.xz /= dot(blend.xz, vec2(1,1));
	// Blend for top
	blend.y = clamp((abs(normal.y) - 0.675) * 80.0, 0.0, 1.0);
	blend.xz *= (1 - blend.y);*/

	/*vec3 blend = pow3(abs(normal.xyz), 4);
	blend /= dot(blend, vec3(1,1,1));*/

	return blending;
}

vec4 triplanarPass(sampler2D texture, float scale, float ts) {
	vec3 blendAxes = getTriPlanarBlend(normalize(Normal));
	//blendAxes = vec3(1.0);

	vec3 scaledWorldPos = WorldPos.xyz / scale;

	vec4 xProjection = texture2D(texture, vec2(scaledWorldPos.y, scaledWorldPos.z) + ts) * blendAxes.x;
	vec4 yProjection = texture2D(texture, vec2(scaledWorldPos.x, scaledWorldPos.z) + ts) * blendAxes.y;
	vec4 zProjection = texture2D(texture, vec2(scaledWorldPos.x, scaledWorldPos.y) + ts) * blendAxes.z;

	vec4 color = xProjection + yProjection + zProjection;
	color.a /= 3.0;
	return color;
}

uniform vec3 LightDir;

void main()
{
	//FragColor = texture(Texture, TexCoord) * u_Tint;
	FragColor = triplanarPass(Texture, 1.0, 1.0);
	FragColor.a = 1.0;

	//float nDotL = dot(normalize(Normal), normalize(vec3(0.5, 0.5, 0.5)));
	//nDotL = max(nDotL, 0.0);

	//FragColor.rgb *= clamp(nDotL, 0.7, 1.0);

	float ao = texture(AO, TexCoord).r;
	//FragColor.rgb *= ao;

#ifdef USE_ALPHA_THRESHOLD
	if(FragColor.a < 0.5)
		discard;
#endif
#if USE_NORMAL_MAP

	vec2 texelSize = 1.0 / vec2(textureSize(NormalMap, 0).xy);

	vec4 normalColor = texture(NormalMap, TexCoord);
	//vec4 normalColor = triplanarPass(NormalMap, 1.0, 1.0);
	//normalColor.xyz = vec3(normalColor.x, 1.0 - normalColor.y, normalColor.z);
	vec3 normalFromTex = normalize(TBN * (normalColor.xyz * 2.0f - 1.0f));
	NormalColor.rgb = normalFromTex * 0.5f + 0.5f;


	float nDotL = dot(normalFromTex, LightDir);
	nDotL = max(nDotL, 0.5);
	FragColor.rgb *= clamp(nDotL, 0.2, 1.0);

#else
	NormalColor.rgb = normalize(Normal) * 0.5f + 0.5f;
#endif
	NormalColor.a = 0.0f;
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