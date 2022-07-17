out vec4 out_color;
in vec2 TexCoord;

uniform sampler2D u_Texture;
uniform sampler2D u_BloomTexture;
uniform sampler2D u_SceneDepth;
uniform sampler2D u_OverlayDepth;

const float decay   = 0.96815;
const float density = 0.946;
const float weight  = 0.687;

layout(std140) uniform PostProcessingFinalBuffer
{
	vec2 sunPos;
	float exposure; //  = 0.3f
	int numSamples;
} uniforms;

void main()
{	
	// Calculate vector from pixel to light source in screen space.
	vec2 texCoord = TexCoord;
	vec2 deltaTexCoord = (texCoord - uniforms.sunPos);
	// Divide by number of samples and scale by control factor.
	deltaTexCoord *= 1.0f / uniforms.numSamples * density;   // Store initial sample.

	vec3 godRayColor = vec3(0, 0, 0); 
	
	// Set up illumination decay factor.    
	float illuminationDecay = 1.0f;
	// Evaluate summation from Equation 3 NUM_SAMPLES iterations.
	for (int i = 0; i < uniforms.numSamples; i++)
	{     
		// Step sample location along ray.
		texCoord -= deltaTexCoord;
		// Retrieve sample at new location. 
		vec2 sunPos  = vec2(uniforms.sunPos.x, uniforms.sunPos.y);
		float hasSun = distance(texCoord, sunPos) < 0.058 ? 0.95 : 0;
		vec3 _sample = min(texture(u_SceneDepth, texCoord).r, texture(u_OverlayDepth, texCoord).r) > .9992 ?
				      vec3(0.10 + hasSun) : 
				      vec3(0, 0, 0); 
		_sample *= illuminationDecay * weight;
		godRayColor += _sample ;
		illuminationDecay *= decay;
	}  

	// merge god rays, bloom, tonemapping
	out_color = vec4(godRayColor * uniforms.exposure * .082, 0) + texture(u_Texture, TexCoord) + texture(u_BloomTexture, TexCoord);
}