#version 330
// Underwater fragment shader with fog effect and MULTIPLE LIGHT SOURCES
// Implements Blinn-Phong lighting model with:
// 1. Directional light (sun)
// 2. Point light (bioluminescent glow)
// 3. Spotlight (diver's flashlight)

uniform sampler2D Texture;
uniform float Transparency;
uniform vec2 TextureOffset;

// Fog
uniform vec3 FogColor;
uniform float FogDensity;

// Camera
uniform vec3 CameraPosition;

// ============ LIGHT 1: Directional Light (Sun) ============
uniform vec3 LightDirection;

// ============ LIGHT 2: Point Light (Bioluminescent) ============
uniform vec3 PointLightPos;
uniform vec3 PointLightColor;
uniform float PointLightIntensity;

// ============ LIGHT 3: Spotlight (Diver's flashlight) ============
uniform vec3 SpotLightPos;
uniform vec3 SpotLightDir;
uniform vec3 SpotLightColor;
uniform float SpotLightCutoff;
uniform float SpotLightIntensity;

// Input from vertex shader
in vec2 texCoord;
in float fogFactor;
in vec3 fragNormal;
in vec3 fragPosition;

out vec4 FragmentColor;

// Blinn-Phong specular calculation
vec3 blinnPhongSpecular(vec3 lightDir, vec3 viewDir, vec3 normal, vec3 lightColor, float shininess) {
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), shininess);
    return lightColor * spec;
}

void main() {
    // Get texture color
    vec4 texColor = texture(Texture, vec2(texCoord.x, 1.0 - texCoord.y) + TextureOffset);
    
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(CameraPosition - fragPosition);
    
    // Material properties
    float shininess = 32.0;
    vec3 specularStrength = vec3(0.3);
    
    // Underwater light attenuation - light gets weaker with depth
    float depth = max(0.0, -fragPosition.y);
    float depthAttenuation = exp(-depth * 0.05);
    
    // ============ LIGHT 1: Directional Light (Sun) ============
    vec3 sunDir = normalize(LightDirection);
    float sunDiffuse = max(dot(normal, sunDir), 0.0);
    vec3 sunDiffuseColor = vec3(1.0, 0.95, 0.85) * sunDiffuse * depthAttenuation * 0.6;
    vec3 sunSpecular = blinnPhongSpecular(sunDir, viewDir, normal, vec3(1.0, 0.95, 0.85), shininess) 
                       * depthAttenuation * 0.3;
    
    // ============ LIGHT 2: Point Light (Bioluminescent glow) ============
    vec3 pointLightVec = PointLightPos - fragPosition;
    float pointDist = length(pointLightVec);
    vec3 pointDir = normalize(pointLightVec);
    
    // Attenuation: 1 / (1 + 0.09*d + 0.032*d^2)
    float pointAtten = PointLightIntensity / (1.0 + 0.09 * pointDist + 0.032 * pointDist * pointDist);
    
    float pointDiffuse = max(dot(normal, pointDir), 0.0);
    vec3 pointDiffuseColor = PointLightColor * pointDiffuse * pointAtten;
    vec3 pointSpecular = blinnPhongSpecular(pointDir, viewDir, normal, PointLightColor, shininess) 
                         * pointAtten * 0.5;
    
    // ============ LIGHT 3: Spotlight (Diver's flashlight) ============
    vec3 spotLightVec = SpotLightPos - fragPosition;
    float spotDist = length(spotLightVec);
    vec3 spotDir = normalize(spotLightVec);
    
    // Check if in spotlight cone
    float spotAngle = dot(spotDir, normalize(-SpotLightDir));
    float spotAtten = 0.0;
    vec3 spotDiffuseColor = vec3(0.0);
    vec3 spotSpecular = vec3(0.0);
    
    if (spotAngle > SpotLightCutoff) {
        // Soft edge falloff
        float spotEdge = (spotAngle - SpotLightCutoff) / (1.0 - SpotLightCutoff);
        spotEdge = clamp(spotEdge, 0.0, 1.0);
        spotAtten = spotEdge * SpotLightIntensity / (1.0 + 0.05 * spotDist + 0.01 * spotDist * spotDist);
        
        float spotDiffuse = max(dot(normal, spotDir), 0.0);
        spotDiffuseColor = SpotLightColor * spotDiffuse * spotAtten;
        spotSpecular = blinnPhongSpecular(spotDir, viewDir, normal, SpotLightColor, shininess) 
                       * spotAtten * 0.5;
    }
    
    // Caustic effect (sun rays through water)
    float caustic = sin(fragPosition.x * 0.5 + fragPosition.z * 0.3) * 
                    sin(fragPosition.z * 0.4 - fragPosition.x * 0.2);
    caustic = caustic * caustic * 0.12 * depthAttenuation;
    
    // Combine all lighting
    vec3 ambient = vec3(0.15, 0.2, 0.3);  // Blue-tinted ambient
    
    vec3 totalDiffuse = sunDiffuseColor + pointDiffuseColor + spotDiffuseColor;
    vec3 totalSpecular = sunSpecular + pointSpecular + spotSpecular;
    
    vec3 litColor = texColor.rgb * (ambient + totalDiffuse) + totalSpecular * specularStrength + caustic;
    
    // Apply fog
    vec3 finalColor = mix(FogColor, litColor, fogFactor);
    
    // ============ HDR TONE MAPPING ============
    // Reinhard tone mapping for HDR
    vec3 hdrColor = finalColor;
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    
    // ============ GAMMA CORRECTION ============
    // Apply gamma correction (sRGB)
    float gamma = 2.2;
    vec3 gammaCorrected = pow(mapped, vec3(1.0 / gamma));
    
    FragmentColor = vec4(gammaCorrected, Transparency);
}
