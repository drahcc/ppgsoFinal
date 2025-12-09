#version 330
// Post-processing vertex shader
// Simple passthrough for full-screen quad

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;

out vec2 texCoord;

void main() {
    texCoord = TexCoord;
    gl_Position = vec4(Position, 1.0);
}
