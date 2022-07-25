#include "Shadows.h"

float LinearShadowSample(sampler2DArrayShadow sampler, int shadowLevel, vec2 coords, float compare, vec2 texelSize)
{
	vec2 pixelPos = coords / texelSize + vec2(0.5);
	vec2 fracPart = fract(pixelPos);
	vec2 startTexel = (pixelPos - fracPart) * texelSize;

	vec4 gatheredTexels = textureGather(sampler, vec3(startTexel, shadowLevel), compare);

	float blTexel = gatheredTexels.w;
	float brTexel = gatheredTexels.z;
	float tlTexel = gatheredTexels.x;
	float trTexel = gatheredTexels.y;

	float mixA = mix(blTexel, tlTexel, fracPart.y);
	float mixB = mix(brTexel, trTexel, fracPart.y);

	return mix(mixA, mixB, fracPart.x);
}

float GetShadowValue(in float shadowBias)
{
	int shadowLevel = 0;
	bool inRange = false;

	float inverseShadow = 1.0;

	for(; shadowLevel < NUM_SHADOW_MAP_LEVELS; shadowLevel++)
	{
		vec3 shadowCoord = ShadowCoords[shadowLevel];
		//shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;
		inRange = shadowCoord.x >= 0 &&
		shadowCoord.y >= 0 &&
		shadowCoord.x <  1 &&
		shadowCoord.y <  1 &&
		shadowCoord.z >= 0 &&
		shadowCoord.z <  1;

		//FragColor.b = shadowCoord.z;

		if(inRange)
		{
			//vec2 texelSize = 1.0f / textureSize(g_ShadowmapTexture, 0).xy;
			//inverseShadow = LinearShadowSample(g_ShadowmapTexture, shadowLevel, shadowCoord.xy, shadowCoord.z - shadowBias, texelSize);
			inverseShadow = texture(g_ShadowmapTexture, vec4(shadowCoord.xy, shadowLevel, shadowCoord.z-shadowBias));
			break;
		}
	}

	/*switch(shadowLevel) {
		case 0: FragColor.rgb = vec3(1,0,0); break;
		case 1: FragColor.rgb = vec3(1,1,0); break;
		case 2: FragColor.rgb = vec3(0,1,0); break;
		case 3: FragColor.rgb = vec3(0,1,1); break;
	}*/

	return inverseShadow;
}