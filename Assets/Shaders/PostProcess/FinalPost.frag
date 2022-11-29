out vec4 FragColor;
in vec2 TexCoord;

#include "tonemappers.glsl"

uniform sampler2D _FinalColor;
uniform sampler2D _FinalBloom;

// Lookup texture tonemap
uniform sampler3D _LutTarget;
uniform vec2 u_LutToneMapData;
uniform bool u_LutToneMapEnabled;
uniform bool u_BasicToneMapEnabled;
uniform uint u_BasicToneMapMode;
uniform float u_BloodEffect;
#define LutColorScale u_LutToneMapData.xxx
#define LutColorOffset u_LutToneMapData.yyy

void main()
{
	vec4 color = texture(_FinalColor, TexCoord);
	color.rgb += texture(_FinalBloom, TexCoord).rgb;

	vec2 uv = TexCoord;
	uv *= 1.0f - uv.yx; // fish eye uv displacement
	float effect = (1.0f-pow(uv.x * uv.y * 15.0f, 0.25f)) * u_BloodEffect;
	color.rgb *= 1.0f - effect; // remove blood effected colors
	color.r += effect; // add red to blood covered areas

	if (u_LutToneMapEnabled)
	{
		color.rgb = texture(_LutTarget, color.rgb * LutColorScale + LutColorOffset).rgb;
	}
	else if (u_BasicToneMapEnabled)
	{
		color.rgb = Tonemap(color.rgb, u_BasicToneMapMode);
	}

	FragColor = color;
}