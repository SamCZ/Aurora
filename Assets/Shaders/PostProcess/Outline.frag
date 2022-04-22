#include "ub_outline.h"

#define OUTLINE_KERNEL_SIZE 2
#define InvTexSize InvMainRTSize.xy
#define OutlineThickness CrossTextureMaskOpacityAndOutlineThickness.y
#define OutlineCrossEnabled OutlineColorAndCrossEnabled.w
#define OutlineColor OutlineColorAndCrossEnabled.rgb
#define CrossColorAlpha CrossColorAndAlpha.a
#define CrossTextureMaskOpacity CrossTextureMaskOpacityAndOutlineThickness.x

out vec4 FraColor;
in vec2 TexCoord;

uniform sampler2D SceneDepthRT;
uniform sampler2D OutlineMaskDepthRT;
uniform sampler2D StripeTexture;

float FetchFloat(sampler2D sampler)
{
	return texelFetch(sampler, ivec2(gl_FragCoord.xy), 0).r;
}

float SampleScreenDepth(vec2 uv)
{
	return texture(SceneDepthRT, uv).r;
}

float SampleMaskOffset(vec2 uv, vec2 offset)
{
	return texture(OutlineMaskDepthRT, TexCoord + offset * InvTexSize).r;
}

void main()
{
	float maskDepth = FetchFloat(OutlineMaskDepthRT);
	float currentMaskVal = maskDepth < 1.0f ? 1.0f : 0.0f;

	float avg = 0;
	float sumWeight = 0;
	float depthPCFCompareAvg = 0;

	for (int i = -OUTLINE_KERNEL_SIZE; i <= OUTLINE_KERNEL_SIZE; i++)
	{
		float weightI = ((OUTLINE_KERNEL_SIZE+1) - abs(i));
		for (int j = -OUTLINE_KERNEL_SIZE; j <= OUTLINE_KERNEL_SIZE; j++)
		{
			float weight = weightI * ((OUTLINE_KERNEL_SIZE+1) - abs(j));
			sumWeight += weight;
			float mdc = SampleMaskOffset(TexCoord, vec2(i, j));
			avg += (mdc < 1.0f ? 1.0f : 0.0f) * weight;

			float depthPCF = (SampleScreenDepth(TexCoord + vec2(i, j) * InvTexSize));
			if(depthPCF < 1e-6f)
			{
				depthPCFCompareAvg += 1 * weight;
				continue;
			}
			depthPCFCompareAvg += (depthPCF < mdc ? 1.0f : -1.0f) * weight;
		}
	}

	avg /= sumWeight;
	depthPCFCompareAvg /= sumWeight;

	float outline = clamp(abs(currentMaskVal - avg), 0, 1) * OutlineThickness;

	vec4 outlineFinal = vec4(OutlineColor, outline);
	float screenDepth = SampleScreenDepth(TexCoord);
	float insideMask = (1.0f - step(maskDepth, screenDepth)) * currentMaskVal;

	if(maskDepth < 1.0f && OutlineCrossEnabled > 0.5f)
		outlineFinal.a = max(outlineFinal.a, 1.0f - clamp(abs(screenDepth - depthPCFCompareAvg), 0.0f, 1.0f));

	float stripeValue = texture(StripeTexture, TexCoord * CrossTextureTexelSize).r;

	if (CrossTextureMaskOpacity > 0.0)
	{
		outlineFinal.rgb = mix(outlineFinal.rgb, stripeValue.rrr * OutlineColor, 1.0f - outlineFinal.a);
		outlineFinal.a += insideMask * CrossTextureMaskOpacity;
	}

	if(1.0f - clamp(depthPCFCompareAvg, 0, 1) < 0.1f)
	{
		outlineFinal.a = min(CrossColorAlpha, outlineFinal.a);
	}

	FraColor = outlineFinal;
}