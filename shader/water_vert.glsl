// Water Surface Vertex Shader
// Creates animated waves on the water surface

#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform float Time;
uniform float WaveHeight;
uniform float WaveFrequency;

out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragPosition;
out float waveOffset;

void main() {
    vec3 pos = Position;
    
    // Create multiple wave layers for realistic water
    float wave1 = sin(pos.x * WaveFrequency + Time * 2.0) * WaveHeight;
    float wave2 = sin(pos.z * WaveFrequency * 0.8 + Time * 1.5) * WaveHeight * 0.7;
    float wave3 = sin((pos.x + pos.z) * WaveFrequency * 0.5 + Time * 2.5) * WaveHeight * 0.5;
    
    pos.y += wave1 + wave2 + wave3;
    waveOffset = (wave1 + wave2 + wave3) / (WaveHeight * 2.2);
    
    // Calculate wave normal (approximation)
    float dx = cos(pos.x * WaveFrequency + Time * 2.0) * WaveHeight * WaveFrequency;
    float dz = cos(pos.z * WaveFrequency * 0.8 + Time * 1.5) * WaveHeight * 0.7 * WaveFrequency * 0.8;
    vec3 waveNormal = normalize(vec3(-dx, 1.0, -dz));
    
    fragPosition = vec3(ModelMatrix * vec4(pos, 1.0));
    fragNormal = mat3(transpose(inverse(ModelMatrix))) * waveNormal;
    fragTexCoord = TexCoord;
    
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(pos, 1.0);
}
