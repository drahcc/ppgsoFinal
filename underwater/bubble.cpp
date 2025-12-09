#include <glm/gtc/matrix_transform.hpp>
#include "bubble.h"
#include "underwater_scene.h"
#include "underwater_camera.h"

#include <shaders/underwater_vert_glsl.h>
#include <shaders/underwater_frag_glsl.h>

// Static resources
std::unique_ptr<ppgso::Mesh> Bubble::mesh;
std::unique_ptr<ppgso::Texture> Bubble::texture;
std::unique_ptr<ppgso::Shader> Bubble::shader;

Bubble::Bubble() {
    // Load shared resources - underwater shader with fog
    if (!shader) shader = std::make_unique<ppgso::Shader>(underwater_vert_glsl, underwater_frag_glsl);
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("bubble/sphere.obj");
    // Use ground texture temporarily until bubbleTexture.bmp is converted to 24-bit
    if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("ground/ground.bmp"));

    // Mark as translucent for depth-sorting
    translucent = true;

    // Small default scale
    scale = {0.1f, 0.1f, 0.1f};
    
    // Random properties
    wobblePhase = static_cast<float>(rand()) / RAND_MAX * 6.28f;
    wobbleFreq = 2.0f + static_cast<float>(rand()) / RAND_MAX * 4.0f;
    riseSpeed = 1.5f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
}

bool Bubble::update(UnderwaterScene& scene, float dt) {
    // Update age
    age += dt;
    
    // Check lifetime or if above water surface
    if (age > lifetime || position.y > 5.0f) {
        return false;  // Remove bubble
    }
    
    // Update wobble
    wobblePhase += wobbleFreq * dt;
    
    // Calculate velocity with wobble
    velocity.y = riseSpeed;
    velocity.x = sin(wobblePhase) * wobbleAmp;
    velocity.z = cos(wobblePhase * 0.7f) * wobbleAmp * 0.5f;
    
    // Update position
    position += velocity * dt;
    
    // Slowly grow as bubble rises (pressure decreases)
    float growthFactor = 1.0f + age * 0.02f;
    scale = glm::vec3(0.1f * growthFactor);
    
    // Fade out near end of lifetime
    if (age > lifetime * 0.8f) {
        transparency = 0.6f * (1.0f - (age - lifetime * 0.8f) / (lifetime * 0.2f));
    }
    
    generateModelMatrix();
    return true;
}

void Bubble::render(UnderwaterScene& scene) {
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    shader->use();
    
    // Set matrices
    shader->setUniform("ProjectionMatrix", scene.camera->projectionMatrix);
    shader->setUniform("ViewMatrix", scene.camera->viewMatrix);
    shader->setUniform("ModelMatrix", modelMatrix);
    
    // Directional light (sun)
    shader->setUniform("LightDirection", scene.lightDirection);
    shader->setUniform("CameraPosition", scene.camera->position);
    
    // Point light (bioluminescent)
    shader->setUniform("PointLightPos", scene.pointLightPos);
    shader->setUniform("PointLightColor", scene.pointLightColor);
    shader->setUniform("PointLightIntensity", scene.pointLightIntensity);
    
    // Spotlight (diver's flashlight)
    shader->setUniform("SpotLightPos", scene.spotLightPos);
    shader->setUniform("SpotLightDir", scene.spotLightDir);
    shader->setUniform("SpotLightColor", scene.spotLightColor);
    shader->setUniform("SpotLightCutoff", scene.spotLightCutoff);
    shader->setUniform("SpotLightIntensity", scene.spotLightIntensity);
    
    // Fog uniforms
    shader->setUniform("FogColor", scene.fogColor);
    shader->setUniform("FogDensity", scene.fogDensity);
    
    // Set texture
    shader->setUniform("Texture", *texture);
    shader->setUniform("Transparency", transparency);
    shader->setUniform("TextureOffset", glm::vec2(0.0f));
    
    mesh->render();
    
    glDisable(GL_BLEND);
}

void Bubble::setRiseSpeed(float speed) {
    riseSpeed = speed;
}

void Bubble::setSize(float size) {
    scale = glm::vec3(size);
}

void Bubble::setLifetime(float life) {
    lifetime = life;
}
