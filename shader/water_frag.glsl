// Water Surface Fragment Shader
// Renders water with sun reflection and transparency

#version 330 core

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPosition;
in float waveOffset;

uniform sampler2D Texture;
uniform vec3 SunDirection;
uniform vec3 SunColor;
uniform vec3 WaterColor;
uniform vec3 CameraPosition;
uniform float Transparency;
uniform float Time;

out vec4 FragColor;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(CameraPosition - fragPosition);
    vec3 sunDir = normalize(SunDirection);
    
    // Determine if viewing from above or below
    bool fromAbove = CameraPosition.y > fragPosition.y;
    
    // Base water colors
    vec3 surfaceColor = vec3(0.1, 0.5, 0.7);      // Brighter blue for surface
    vec3 deepColor = vec3(0.0, 0.2, 0.4);         // Darker blue below
    vec3 skyReflect = vec3(0.4, 0.6, 0.8);        // Sky reflection color
    
    // Mix based on wave height
    vec3 baseColor = mix(deepColor, surfaceColor, 0.5 + waveOffset * 0.5);
    
    // Diffuse lighting from sun
    float diffuse = max(dot(normal, sunDir), 0.0);
    
    // Specular reflection (sun glitter on water) - only from above
    vec3 specular = vec3(0.0);
    if (fromAbove) {
        vec3 reflectDir = reflect(-sunDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
        specular = SunColor * spec * 1.5;
    }
    
    // Fresnel effect - more reflection at grazing angles
    float fresnel = pow(1.0 - max(dot(viewDir, normal), 0.0), 4.0);
    
    // Combine colors
    vec3 waterSurface = baseColor * (0.5 + diffuse * 0.5);
    
    // Add sky reflection from above
    if (fromAbove) {
        waterSurface = mix(waterSurface, skyReflect, fresnel * 0.6);
        waterSurface += specular;
    } else {
        // From below - lighter, more cyan
        waterSurface = mix(waterSurface, vec3(0.2, 0.6, 0.8), fresnel * 0.4);
    }
    
    // Add subtle animation patterns
    float pattern = sin(fragTexCoord.x * 30.0 + Time * 0.5) * sin(fragTexCoord.y * 30.0 + Time * 0.3);
    waterSurface += vec3(pattern * 0.03);
    
    // Transparency - more opaque when viewed at angle
    float alpha = fromAbove ? mix(0.6, 0.9, fresnel) : mix(0.5, 0.85, fresnel);
    
    FragColor = vec4(waterSurface, alpha * Transparency);
}
