out vec4 out_color;
in vec2 TexCoord;

uniform sampler2D u_Texture;
uniform sampler2D u_BloomTexture;
uniform sampler2D u_SceneDepth;
uniform sampler2D u_OverlayDepth;

const float decay = 0.95815;
const float density  = 0.906;
const float weight  = 0.587;

layout(std140) uniform PostProcessingFinalBuffer
{
	vec2 sunPos;
	float exposure; //  = 0.3f
	int numSamples;
} uniforms;

#define NVIDIA_IMPL 1

void main()
{	
#ifdef NVIDIA_IMPL 
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
		vec3 _sample = min(texture(u_SceneDepth, texCoord).r, texture(u_OverlayDepth, texCoord).r) > .9992 ?
				      vec3(1, 1, 1) : 
				      vec3(0, 0, 0); 
		// Apply sample attenuation scale/decay factors.
		_sample *= illuminationDecay * weight;
		// Accumulate combined color.
		godRayColor += _sample ;
		// Update exponential decay factor.
		illuminationDecay *= decay;
	}  

	// merge god rays, bloom, tonemapping
	out_color = vec4(godRayColor * uniforms.exposure * .05, 0) + texture(u_Texture, TexCoord) + texture(u_BloomTexture, TexCoord);
#else
	vec2 deltaTextCoord = vec2(TexCoord - uniforms.sunPos);
    vec2 texCoord = TexCoord;
	vec2 sunPos = uniforms.sunPos;
	
	deltaTextCoord *= 1.0 /  float(uniforms.numSamples) * density;
    float illuminationDecay = 1.0;
	vec4 godRayColor = vec4(0);
	
    for (int i = 0; i < uniforms.numSamples; i++)
    {
        texCoord -= deltaTextCoord;
    
		vec4 samp = distance(texCoord, sunPos) < 0.038 && // has sun ? 
		   // check sun is blocking by any other object
		   // I don't even know why I'm using .9992  
		   min(texture(u_SceneDepth, texCoord).r, texture(u_OverlayDepth, texCoord).r) > .9992 ? 
		   vec4(1, 1, 1, 1) : 
		   vec4(0, 0, 0, 1); 
		
        samp *= illuminationDecay * weight;
        godRayColor += samp;
        illuminationDecay *= decay;
    }
	// merge god rays, bloom, tonemapping
	out_color = godRayColor * uniforms.exposure * .15 + texture(u_Texture, TexCoord) + texture(u_BloomTexture, TexCoord);
#endif
}