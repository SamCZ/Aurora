out vec4 out_color;
in vec2 TexCoord;

uniform sampler2D u_Texture;

void main()
{
    out_color = texture(u_Texture, TexCoord);
}