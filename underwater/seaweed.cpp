#include <glm/gtc/matrix_transform.hpp>
#include "seaweed.h"
#include "underwater_scene.h"
#include "underwater_camera.h"

#include <shaders/underwater_vert_glsl.h>
#include <shaders/underwater_frag_glsl.h>

// Static resources
std::unique_ptr<ppgso::Mesh> Seaweed::mesh;
std::unique_ptr<ppgso::Texture> Seaweed::texture;
std::unique_ptr<ppgso::Shader> Seaweed::shader;

Seaweed::Seaweed() {
    // Load shared resources - underwater shader with fog
    if (!shader) shader = std::make_unique<ppgso::Shader>(underwater_vert_glsl, underwater_frag_glsl);
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("seaweed/maya2sketchfab.obj");
    if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("seaweed/abstract-solid-shining-yellow-gradient-studio-wall-room-background.bmp"));

    // Default scale
    scale = {0.5f, 0.5f, 0.5f};
    
    // Random sway phase
    swayPhase = static_cast<float>(rand()) / RAND_MAX * 6.28f;
    swaySpeed = 0.3f + static_cast<float>(rand()) / RAND_MAX * 0.3f;
    swayAmplitude = 0.05f;
    
    // No base rotation - keep model as-is
    baseRotation = glm::vec3(0.0f);
}

bool Seaweed::update(UnderwaterScene& scene, float dt) {
    swayPhase += swaySpeed * dt;
    
    // Very gentle sway
    rotation.x = sin(swayPhase) * swayAmplitude;
    rotation.z = sin(swayPhase * 0.7f + 1.0f) * swayAmplitude * 0.5f;
    // Y rotation set at spawn time
    
    generateModelMatrix();
    return true;
}

void Seaweed::render(UnderwaterScene& scene) {
    // Disable culling for plants (two-sided leaves)
    glDisable(GL_CULL_FACE);
    
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
    shader->setUniform("Transparency", 1.0f);
    shader->setUniform("TextureOffset", glm::vec2(0.0f));
    
    mesh->render();
    
    glEnable(GL_CULL_FACE);
}
