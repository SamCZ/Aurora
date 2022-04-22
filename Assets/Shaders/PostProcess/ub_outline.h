#include "../ps_common.h"

uniformbuffer OutlineGPUDesc
{
	vec2 MainRTSize;
	vec2 InvMainRTSize;
	vec2 CrossTextureTexelSize;
	vec2 CrossTextureMaskOpacityAndOutlineThickness;
	vec4 OutlineColorAndCrossEnabled;	// Outline color RGB, CrossOutlineEnabled
	vec4 CrossColorAndAlpha;	// Cross color RGB, Intersection opacity
};