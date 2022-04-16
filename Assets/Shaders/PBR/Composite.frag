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
	float dist = length(diff);
	vec3 N = normalize(diff);
	float radius = light.ColorRadius.w;
	float att = clamp(1.0 - dist*dist/(radius*radius), 0.0, 1.0);
	att *= att;

	float nDotL = dot(N, normal);
	nDotL = max(nDotL, 0.2f);

	return color * nDotL * att * light.ColorRadius.rgb * light.PositionIntensity.w;
}

void main()
{
	if (true)
	{
		//FragColor = texture(NormalsRT, TexCoord);
		//return;
	}

	vec4 albedo = texture(AlbedoRT, TexCoord);
	vec3 normalColor = texture(NormalsRT, TexCoord).rgb;
	vec3 normals = normalColor * 2.0f - 1.0f;

	if (length(normalColor) == 0.0)
	{
		FragColor = albedo;
		return;
	}

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