#version 330
// Cubemap Skybox Fragment Shader
// Procedural sky with sun, gradient, and clouds

in vec3 TexCoords;
out vec4 FragmentColor;

uniform float Time;
uniform vec3 SunDirection;

// Simple noise function for clouds
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 4; i++) {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void main() {
    vec3 dir = normalize(TexCoords);
    
    // Sky gradient based on Y direction (up/down)
    float y = dir.y;
    
    // Colors for sky gradient
    vec3 horizonColor = vec3(0.7, 0.8, 0.95);   // Light blue at horizon
    vec3 zenithColor = vec3(0.2, 0.4, 0.8);     // Deep blue at top
    vec3 nadirColor = vec3(0.0, 0.1, 0.3);      // Dark blue below
    
    vec3 skyColor;
    if (y > 0.0) {
        // Above horizon - blend to zenith
        float t = pow(y, 0.5);
        skyColor = mix(horizonColor, zenithColor, t);
    } else {
        // Below horizon - blend to nadir (underwater look)
        float t = pow(-y, 0.7);
        skyColor = mix(horizonColor, nadirColor, t);
    }
    
    // Sun
    vec3 sunDir = normalize(SunDirection);
    float sunDot = dot(dir, sunDir);
    
    // Sun disk
    float sunSize = 0.995;
    if (sunDot > sunSize) {
        float sunIntensity = (sunDot - sunSize) / (1.0 - sunSize);
        vec3 sunColor = vec3(1.0, 0.95, 0.8);
        skyColor = mix(skyColor, sunColor, sunIntensity * sunIntensity);
    }
    
    // Sun glow
    float glowSize = 0.9;
    if (sunDot > glowSize) {
        float glow = (sunDot - glowSize) / (1.0 - glowSize);
        vec3 glowColor = vec3(1.0, 0.7, 0.4);
        skyColor += glowColor * glow * glow * 0.3;
    }
    
    // Clouds (only above horizon)
    if (y > 0.0) {
        // Project direction onto a plane for cloud UV
        vec2 cloudUV = dir.xz / (y + 0.1) * 2.0;
        cloudUV += Time * 0.02;  // Slowly moving clouds
        
        float cloudNoise = fbm(cloudUV * 1.5);
        float cloudCoverage = 0.4;
        float clouds = smoothstep(cloudCoverage, cloudCoverage + 0.3, cloudNoise);
        
        // Cloud color (white with slight blue tint)
        vec3 cloudColor = vec3(0.95, 0.95, 1.0);
        
        // Fade clouds near horizon
        float horizonFade = smoothstep(0.0, 0.3, y);
        clouds *= horizonFade;
        
        skyColor = mix(skyColor, cloudColor, clouds * 0.7);
    }
    
    // Sunset/sunrise tint near horizon
    float horizonGlow = exp(-abs(y) * 5.0);
    vec3 sunsetColor = vec3(1.0, 0.5, 0.3);
    float sunsetAmount = max(0.0, dot(dir, vec3(sunDir.x, 0.0, sunDir.z)));
    skyColor += sunsetColor * horizonGlow * sunsetAmount * 0.3;
    
    FragmentColor = vec4(skyColor, 1.0);
}
