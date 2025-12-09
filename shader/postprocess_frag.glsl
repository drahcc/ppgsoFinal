#version 330
// Post-processing fragment shader with multiple effects
// Supports: Grayscale, Blur, Sharpen, Edge Detection, Bloom, Vignette

uniform sampler2D Texture;
uniform int EffectType;  // 0=none, 1=grayscale, 2=blur, 3=sharpen, 4=edge, 5=bloom, 6=vignette
uniform float Time;

in vec2 texCoord;
out vec4 FragmentColor;

// Gaussian blur kernel 5x5
float gaussianKernel[25] = float[](
    1.0,  4.0,  6.0,  4.0, 1.0,
    4.0, 16.0, 24.0, 16.0, 4.0,
    6.0, 24.0, 36.0, 24.0, 6.0,
    4.0, 16.0, 24.0, 16.0, 4.0,
    1.0,  4.0,  6.0,  4.0, 1.0
);

// Sharpen kernel
float sharpenKernel[9] = float[](
    0.0, -1.0,  0.0,
   -1.0,  5.0, -1.0,
    0.0, -1.0,  0.0
);

// Edge detection (Sobel)
float sobelX[9] = float[](
   -1.0, 0.0, 1.0,
   -2.0, 0.0, 2.0,
   -1.0, 0.0, 1.0
);

float sobelY[9] = float[](
   -1.0, -2.0, -1.0,
    0.0,  0.0,  0.0,
    1.0,  2.0,  1.0
);

vec4 applyKernel3x3(float[9] kernel) {
    vec4 color = vec4(0.0);
    vec2 texelSize = 1.0 / textureSize(Texture, 0);
    int index = 0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            vec2 offset = vec2(x, y) * texelSize;
            color += kernel[index++] * texture(Texture, texCoord + offset);
        }
    }
    return color;
}

vec4 applyGaussianBlur() {
    vec4 color = vec4(0.0);
    vec2 texelSize = 1.0 / textureSize(Texture, 0);
    int index = 0;
    float totalWeight = 256.0;
    
    for (int y = -2; y <= 2; y++) {
        for (int x = -2; x <= 2; x++) {
            vec2 offset = vec2(x, y) * texelSize;
            color += gaussianKernel[index++] * texture(Texture, texCoord + offset);
        }
    }
    return color / totalWeight;
}

vec4 applyEdgeDetection() {
    vec4 gx = applyKernel3x3(sobelX);
    vec4 gy = applyKernel3x3(sobelY);
    return sqrt(gx * gx + gy * gy);
}

vec4 applyBloom() {
    vec4 color = texture(Texture, texCoord);
    vec4 blur = applyGaussianBlur();
    
    // Extract bright areas
    float brightness = dot(blur.rgb, vec3(0.2126, 0.7152, 0.0722));
    vec4 brightColor = blur * smoothstep(0.5, 1.0, brightness);
    
    // Add bloom to original
    return color + brightColor * 0.5;
}

vec4 applyVignette(vec4 color) {
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(texCoord, center);
    float vignette = smoothstep(0.8, 0.4, dist);
    return vec4(color.rgb * vignette, color.a);
}

void main() {
    vec4 color = texture(Texture, texCoord);
    
    if (EffectType == 0) {
        // No effect
        FragmentColor = color;
    }
    else if (EffectType == 1) {
        // Grayscale
        float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        FragmentColor = vec4(vec3(gray), color.a);
    }
    else if (EffectType == 2) {
        // Gaussian Blur
        FragmentColor = applyGaussianBlur();
    }
    else if (EffectType == 3) {
        // Sharpen
        FragmentColor = applyKernel3x3(sharpenKernel);
    }
    else if (EffectType == 4) {
        // Edge Detection
        FragmentColor = applyEdgeDetection();
    }
    else if (EffectType == 5) {
        // Bloom
        FragmentColor = applyBloom();
    }
    else if (EffectType == 6) {
        // Vignette
        FragmentColor = applyVignette(color);
    }
    else if (EffectType == 7) {
        // Underwater distortion effect
        vec2 distortedCoord = texCoord;
        distortedCoord.x += sin(texCoord.y * 20.0 + Time * 2.0) * 0.003;
        distortedCoord.y += cos(texCoord.x * 20.0 + Time * 2.0) * 0.003;
        vec4 distortedColor = texture(Texture, distortedCoord);
        // Add slight blue tint and vignette
        distortedColor.rgb = mix(distortedColor.rgb, vec3(0.0, 0.3, 0.5), 0.1);
        FragmentColor = applyVignette(distortedColor);
    }
    else {
        FragmentColor = color;
    }
}
