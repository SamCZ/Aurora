out vec4 FragColor;
in vec2 TexCoord;

layout(std140) uniform BloomDesc
{
	ivec2 screenSize; 
	int size;
	int operation;
	float treshold;
} bloomDesc;

layout(binding = 1) uniform sampler2D u_Texture;
layout(binding = 2) uniform sampler2D u_BloomTexture;

#define EPSILON  1.0e-4

#define OPERATION_FragPrefilter13  1
#define OPERATION_FragPrefilter4   2
#define OPERATION_FragDownsample13 3
#define OPERATION_FragDownsample4  4
#define OPERATION_FragUpsampleTent 5
#define OPERATION_FragUpsampleBox  6

float Max3(float a, float b, float c)
{
	return max(max(a, b), c);
}

vec4 SafeHDR(in vec4 c)
{
	return min(c, 32.0f); // 32 is max value that we desire for hdr
}

vec4 QuadraticThreshold(in vec4 color, float threshold, vec3 curve)
{
	// Pixel brightness
	float br = Max3(color.r, color.g, color.b);

	// Under-threshold part: quadratic curve
	float rq = clamp(br - curve.x, 0.0, curve.y);
	rq = curve.z * rq * rq;
	
	// Combine and apply the brightness response curve.
	color *= max(rq, br - threshold) / max(br, EPSILON);

	return color;
}

// 9-tap bilinear upsampler (tent filter)
vec4 UpsampleTent(sampler2D tex, vec2 texCoord, vec2 texelSize)
{
	ivec4 d = ivec4(1, 1, -1, 0);

	vec4 s;
	s  = texture(tex, texCoord - (texelSize * vec2(d.xy)));
	s += texture(tex, texCoord - (texelSize * vec2(d.wy))) * 2.0;
	s += texture(tex, texCoord - (texelSize * vec2(d.zy)));

	s += texture(tex, texCoord + (texelSize * vec2(d.zw))) * 2.0;
	s += texture(tex, texCoord) * 4.0;
	s += texture(tex, texCoord + (texelSize * vec2(d.xw))) * 2.0;

	s += texture(tex, texCoord + (texelSize * vec2(d.zy)));
	s += texture(tex, texCoord + (texelSize * vec2(d.wy))) * 2.0;
	s += texture(tex, texCoord + (texelSize * vec2(d.xy)));

	return s * (1.0 / 16.0);
}

// Standard box filtering
vec4 UpsampleBox(sampler2D tex, vec2 texCoord, vec2 texelSize)
{
	ivec4 d = ivec4(-1, -1, 1, 1);

	vec4 s;
	s  = texture(tex, texCoord * 2.0 + (texelSize * vec2(d.xy)));
	s += texture(tex, texCoord * 2.0 + (texelSize * vec2(d.zy)));
	s += texture(tex, texCoord * 2.0 + (texelSize * vec2(d.xw)));
	s += texture(tex, texCoord * 2.0 + (texelSize * vec2(d.zw)));

	return s * (1.0 / 4.0);
}
// Standard box filtering
vec4 DownsampleBox4Tap(sampler2D tex, vec2 texCoord, vec2 texelSize)
{
	ivec4 d = ivec4(-1.0, -1.0, 1.0, 1.0);

	vec4 s;
	s  = texture(tex, texCoord + (texelSize * vec2(d.xy)));
	s += texture(tex, texCoord + (texelSize * vec2(d.zy)));
	s += texture(tex, texCoord + (texelSize * vec2(d.xw)));
	s += texture(tex, texCoord + (texelSize * vec2(d.zw)));

	return s * (1.0 / 4.0);
}

// DownsampleBox13Tap
vec4 DownsampleBox13Tap(sampler2D tex, vec2 texCoord, vec2 texelSize)
{
	vec4 A = texture(tex, texCoord + (texelSize * vec2(-2.0, -2.0)));
	vec4 B = texture(tex, texCoord + (texelSize * vec2( 0.0, -2.0)));
	vec4 C = texture(tex, texCoord + (texelSize * vec2( 2.0, -2.0)));
	vec4 D = texture(tex, texCoord + (texelSize * vec2(-1.0, -1.0)));
	vec4 E = texture(tex, texCoord + (texelSize * vec2( 1.0, -1.0)));
	vec4 F = texture(tex, texCoord + (texelSize * vec2(-2.0, 0.0)));
	vec4 G = texture(tex, texCoord + (texelSize * vec2( 0.0, 0.0)));
	vec4 H = texture(tex, texCoord + (texelSize * vec2( 2.0, 0.0)));
	vec4 I = texture(tex, texCoord + (texelSize * vec2(-1.0, 1.0)));
	vec4 J = texture(tex, texCoord + (texelSize * vec2( 1.0, 1.0)));
	vec4 K = texture(tex, texCoord + (texelSize * vec2(-2.0, 2.0)));
	vec4 L = texture(tex, texCoord + (texelSize * vec2( 0.0, 2.0)));
	vec4 M = texture(tex, texCoord + (texelSize * vec2( 2.0, 2.0)));

	vec2 div = (1.0 / 4.0) * vec2(0.5, 0.125);

	vec4 o = (D + E + I + J) * div.x;
	o += (A + B + G + F) * div.y;
	o += (B + C + H + G) * div.y;
	o += (F + G + L + K) * div.y;
	o += (G + H + M + L) * div.y;
	o.a = 1.0;
	return o;
}

vec4 Prefilter(float Treshold, vec4 color, vec2 uv)
{
	// static const float Treshold = 0.65;
	const float knee = 0.33f;
	// x: threshold value (linear), y: threshold - knee, z: knee * 2, w: 0.25 / knee
	vec4 _Threshold = vec4(Treshold, Treshold - knee, knee * 2.0, 0.25 / knee);
	color = QuadraticThreshold(color, _Threshold.x, _Threshold.yzw);
	return color;
}

void main()
{
	vec2 texelSize = vec2(1.0, 1.0) / bloomDesc.screenSize;
	vec2 texCoord = TexCoord * bloomDesc.size;

	if (bloomDesc.operation == OPERATION_FragPrefilter13)
	{
		vec4 color = DownsampleBox13Tap(u_Texture, texCoord, texelSize);
		FragColor = Prefilter(bloomDesc.treshold, SafeHDR(color), texCoord );
	}
	else if (bloomDesc.operation == OPERATION_FragPrefilter4)
	{
		vec4 color = DownsampleBox4Tap(u_Texture, texCoord, texelSize);
		FragColor = Prefilter(bloomDesc.treshold, SafeHDR(color), texCoord);
	}
	else if (bloomDesc.operation == OPERATION_FragDownsample13)
	{
		FragColor = DownsampleBox13Tap(u_Texture, texCoord, texelSize);
	}	
	else if (bloomDesc.operation == OPERATION_FragDownsample4)
	{
		FragColor = DownsampleBox4Tap(u_Texture, texCoord, texelSize);
	}	
	else if (bloomDesc.operation == OPERATION_FragUpsampleTent)
	{
		vec4 bloom = UpsampleTent(u_Texture, texCoord, texelSize);
		FragColor = bloom + texture(u_BloomTexture, texCoord);
	}
	else if (bloomDesc.operation == OPERATION_FragUpsampleBox)
	{
		vec4 bloom = UpsampleBox(u_Texture, texCoord * 0.5, texelSize);
		FragColor = bloom + texture(u_BloomTexture, texCoord);
	}
}