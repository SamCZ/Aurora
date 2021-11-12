#pragma once

namespace Aurora::Shaders
{
const char* INTERNAL_SHADER_BLIT_VS = R"(
out vec2 TexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() {
    uint u = ~gl_VertexID & 1;
    uint v = (gl_VertexID >> 1) & 1;

    gl_Position = vec4(vec2(u, v) * 2 - 1, 0, 1);
    gl_Position.y = -gl_Position.y;

    TexCoord = vec2(u, 1.0f - float(v));
}
)";

const char* INTERNAL_SHADER_BLIT_PS = R"(
in vec2 TexCoord;
layout(location = 0) out vec4 FragColor;

uniform sampler2D Texture;

void main() {
    FragColor = texture(Texture, TexCoord);
}
)";
}