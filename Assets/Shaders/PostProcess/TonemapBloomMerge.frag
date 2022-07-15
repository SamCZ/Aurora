out vec4 out_color;
in vec2 TexCoord;

uniform sampler2D u_Texture;
uniform sampler2D u_BloomTexture;

void main()
{
    out_color = texture(u_Texture, TexCoord) + texture(u_BloomTexture, TexCoord);
}