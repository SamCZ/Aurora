#pragma once

namespace Aurora::Shaders
{
const char* INTERNAL_SHADER_CLEAR_VS = R"(
out gl_PerVertex
{
    vec4 gl_Position;
};

void main() {
    uint u = ~gl_VertexID & 1;
    uint v = (gl_VertexID >> 1) & 1;

    gl_Position = vec4(vec2(u, v) * 2 - 1, 0, 1);
    gl_Position.y = -gl_Position.y;
}
)";

const char* INTERNAL_SHADER_CLEAR_PS = R"(
layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = vec4(0, 0, 0, 0);
}
)";
}