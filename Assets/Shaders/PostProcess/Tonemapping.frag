// #include "tonemappers.glsl"

in vec2 TexCoord;
out vec4 colorOut;

uniform sampler2D u_Texture;

vec3 aces(in vec3 x) {
  const float a = 2.51; const float b = 0.03;
  const float c = 2.43; const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
	  colorOut = vec4(aces(texture(u_Texture, TexCoord).rgb), 1);
}
