#pragma once
#include "common.h"


#if !defined(SHADER_ENGINE_SIDE)
// Converts a color from linear light gamma to sRGB gamma
vec4 fromLinear(vec4 linearRGB)
{
	bvec3 cutoff = lessThan(linearRGB.rgb, vec3(0.0031308));
	vec3 higher = vec3(1.055)*pow(linearRGB.rgb, vec3(1.0/2.4)) - vec3(0.055);
	vec3 lower = linearRGB.rgb * vec3(12.92);

	return vec4(mix(higher, lower, vec3(cutoff)), linearRGB.a);
}

// Converts a color from sRGB gamma to linear light gamma
vec4 toLinear(vec4 sRGB)
{
	bvec3 cutoff = lessThan(sRGB.rgb, vec3(0.04045));
	vec3 higher = pow((sRGB.rgb + vec3(0.055))/vec3(1.055), vec3(2.4));
	vec3 lower = sRGB.rgb/vec3(12.92);

	return vec4(mix(higher, lower, vec3(cutoff)), sRGB.a);
}
vec3 fromLinear(vec3 linearRGB)
{
	bvec3 cutoff = lessThan(linearRGB.rgb, vec3(0.0031308));
	vec3 higher = vec3(1.055)*pow(linearRGB.rgb, vec3(1.0/2.4)) - vec3(0.055);
	vec3 lower = linearRGB.rgb * vec3(12.92);

	return mix(higher, lower, vec3(cutoff));
}

// Converts a color from sRGB gamma to linear light gamma
vec3 toLinear(vec3 sRGB)
{
	bvec3 cutoff = lessThan(sRGB.rgb, vec3(0.04045));
	vec3 higher = pow((sRGB.rgb + vec3(0.055))/vec3(1.055), vec3(2.4));
	vec3 lower = sRGB.rgb/vec3(12.92);

	return mix(higher, lower, vec3(cutoff));
}
#endif

const float near_plane = 0.1f;
const float far_plane = 2000.0f;

float linearize_depth(float d,float zNear,float zFar)
{
	return zNear * zFar / (zFar + d * (zNear - zFar));
}

float LinearizeDepth(float depth) {
	return linearize_depth(depth, near_plane, far_plane);
}