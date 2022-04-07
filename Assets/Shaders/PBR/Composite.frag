#include "Composite.h"

out vec4 FragColor;

in vec2 TexCoord;

layout(binding = 0) uniform sampler2D AlbedoRT;
layout(binding = 1) uniform sampler2D NormalsRT;
layout(binding = 1) uniform sampler2D DepthRT;

vec3 GetSceneWorldPos()
{
	ivec2 texel = ivec2(gl_FragCoord.xy);
	vec4 clipSpaceLocation;
	clipSpaceLocation.xy = TexCoord * 2.0f - 1.0f;
	clipSpaceLocation.z = texelFetch(DepthRT, texel, 0).r;
	clipSpaceLocation.w = 1.0f;

	vec4 homogenousLocation = InvProjectionView * clipSpaceLocation;
	return homogenousLocation.xyz / homogenousLocation.w;
}


vec3 ApplyDirectionalLight(DirectionalLightGPU light, vec3 color, vec3 normal)
{
	float nDotL = dot(light.DirectionIntensity.xyz, normal);
	nDotL = max(nDotL, 0.2f);

	return color * nDotL * light.Color.rgb * light.DirectionIntensity.w;
}

vec3 ApplyPointLight(PointLightGPU light, vec3 color, vec3 normal, vec3 worldPos)
{
	vec3 diff = light.PositionIntensity.xyz - worldPos;
	float D = length(diff);
	vec3 N = normalize(diff);
	float L = light.ColorRadius.w / D;

	float nDotL = dot(N, normal);

	return color * nDotL * L * light.ColorRadius.rgb * light.PositionIntensity.w;
}

void main()
{
	vec4 albedo = texture(AlbedoRT, TexCoord);
	vec3 normals = texture(NormalsRT, TexCoord).rgb * 2.0f - 1.0f;
	vec3 worldPos = GetSceneWorldPos();

	vec3 color = vec3(0.0f);

	for (uint i = 0; i < DirLightCount; ++i)
	{
		color += ApplyDirectionalLight(DirLights[i], albedo.rgb, normals);
	}

	for (uint i = 0; i < PointLightCount; ++i)
	{
		color += ApplyPointLight(PointLights[i], albedo.rgb, normals, worldPos);
	}

	FragColor.rgb = color;
	FragColor.a = 1.0f;
}