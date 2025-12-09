#version 330
// Cubemap Skybox Vertex Shader

layout(location = 0) in vec3 Position;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

out vec3 TexCoords;

void main() {
    // Use position as texture coordinates for cubemap lookup
    TexCoords = Position;
    
    // Remove translation from view matrix (skybox always centered on camera)
    mat4 viewNoTranslation = mat4(mat3(ViewMatrix));
    
    vec4 pos = ProjectionMatrix * viewNoTranslation * vec4(Position, 1.0);
    
    // Set z = w so depth is always 1.0 (far plane) after perspective divide
    gl_Position = pos.xyww;
}
