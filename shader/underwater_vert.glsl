#version 330
// Underwater vertex shader with fog support

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

// Output to fragment shader
out vec2 texCoord;
out float fogFactor;
out vec3 fragNormal;
out vec3 fragPosition;

// Fog parameters
uniform float FogDensity;  // How thick the fog is (0.01 - 0.05 typical)
uniform float FogStart;    // Where fog starts
uniform float FogEnd;      // Where fog is fully opaque

void main() {
    texCoord = TexCoord;
    
    // Calculate world position
    vec4 worldPos = ModelMatrix * vec4(Position, 1.0);
    vec4 viewPos = ViewMatrix * worldPos;
    
    // Pass world position to fragment shader for lighting
    fragPosition = worldPos.xyz;
    
    // Calculate distance from camera (in view space, z is depth)
    float distance = length(viewPos.xyz);
    
    // Exponential fog - more realistic for underwater
    // fogFactor = 1.0 means no fog, 0.0 means full fog
    fogFactor = exp(-FogDensity * distance);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    // Pass normal for potential lighting
    fragNormal = mat3(transpose(inverse(ModelMatrix))) * Normal;
    
    gl_Position = ProjectionMatrix * viewPos;
}
