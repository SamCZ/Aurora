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
#define LutColorScale u_LutToneMapData.xxx
#define LutColorOffset u_LutToneMapData.yyy


void main()
{
	vec4 color = texture(_FinalColor, TexCoord);
	color.rgb += texture(_FinalBloom, TexCoord).rgb;

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