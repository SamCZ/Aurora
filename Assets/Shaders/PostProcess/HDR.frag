#include "../ps_common.h"
#include "tonemappers.glsl"

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D SceneHRDTexture;
uniform sampler2D BloomTexture;
#ifdef USE_OUTLINE
uniform sampler2D OutlineTexture;
#endif

uniform float BloomIntensity;

void main()
{
	vec4 sceneColor = texelFetch(SceneHRDTexture, ivec2(gl_FragCoord.xy), 0);
	vec4 bloomColor = texture(BloomTexture, TexCoord);

	const float gamma = 2.2;

	FragColor = sceneColor;
	FragColor.rgb += bloomColor.rgb * BloomIntensity;
	//FragColor.rgb = Reinhard(FragColor.rgb);
	//FragColor.a = sceneColor.a;

	//FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / gamma));
#ifdef USE_OUTLINE
	vec4 outlineColor = texelFetch(OutlineTexture, ivec2(gl_FragCoord.xy), 0);
	FragColor.rgb = mix(FragColor.rgb, outlineColor.rgb, outlineColor.a);
#endif
	FragColor = fromLinear(FragColor);
	FragColor.a = sceneColor.a;
}