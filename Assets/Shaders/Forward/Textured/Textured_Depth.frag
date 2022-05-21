in vec2 TexCoord;

uniform sampler2D Texture;

void main()
{
#ifdef USE_ALPHA_THRESHOLD
	if(texture(Texture, TexCoord).a < 0.5)
		discard;
#endif
}