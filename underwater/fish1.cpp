#include <glm/gtc/matrix_transform.hpp>
#include "fish1.h"
#include "underwater_scene.h"
#include "underwater_camera.h"

#include <shaders/underwater_vert_glsl.h>
#include <shaders/underwater_frag_glsl.h>

// Static resources
std::unique_ptr<ppgso::Mesh> Fish1::mesh;
std::unique_ptr<ppgso::Texture> Fish1::texture;
std::unique_ptr<ppgso::Shader> Fish1::shader;

Fish1::Fish1() {
    // Load shared resources - underwater shader with fog
    if (!shader) shader = std::make_unique<ppgso::Shader>(underwater_vert_glsl, underwater_frag_glsl);
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("fish1/fish.obj");
    if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("fish1/fish_24bit.bmp"));

    // Default scale - adjust based on model size
    scale = {0.3f, 0.3f, 0.3f};
    
    // Random swim phase for variety
    swimPhase = static_cast<float>(rand()) / RAND_MAX * 6.28f;
    
    // Random initial direction
    float angle = static_cast<float>(rand()) / RAND_MAX * 6.28f;
    currentYaw = angle;
    targetYaw = angle;
    direction = glm::vec3(sin(angle), 0, cos(angle));
    
    // Random turn timing
    nextTurnTime = 3.0f + static_cast<float>(rand()) / RAND_MAX * 4.0f;
}

bool Fish1::update(UnderwaterScene& scene, float dt) {
    // Update swim animation (tail wagging)
    swimPhase += speed * dt * 2.0f;
    
    // Update turn timer
    turnTimer += dt;
    
    // Time for a new turn?
    if (turnTimer >= nextTurnTime) {
        turnTimer = 0.0f;
        nextTurnTime = 3.0f + static_cast<float>(rand()) / RAND_MAX * 4.0f;
        
        // Check if too far from school center
        glm::vec3 toCenter = schoolCenter - position;
        float distToCenter = glm::length(toCenter);
        
        if (distToCenter > schoolRadius) {
            // Turn back toward school
            targetYaw = atan2(toCenter.x, toCenter.z);
        } else {
            // Random turn within school area
            targetYaw = currentYaw + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f;
        }
    }
    
    // Smoothly turn toward target
    float yawDiff = targetYaw - currentYaw;
    // Normalize angle difference
    while (yawDiff > 3.14159f) yawDiff -= 6.28318f;
    while (yawDiff < -3.14159f) yawDiff += 6.28318f;
    
    currentYaw += yawDiff * turnSpeed * dt;
    
    // Update direction based on yaw
    direction = glm::vec3(sin(currentYaw), 0, cos(currentYaw));
    
    // Move forward
    position += direction * speed * dt;
    
    // Keep fish at reasonable depth (slight vertical variation)
    float targetY = schoolCenter.y + sin(swimPhase * 0.3f) * 1.0f;
    position.y += (targetY - position.y) * dt * 0.5f;
    
    // Rotation - face swimming direction with tail wag
    rotation.y = -currentYaw;  // Face forward
    rotation.z = sin(swimPhase) * tailSwayAmount;  // Tail wag
    rotation.x = sin(swimPhase * 0.5f) * 0.05f;  // Slight pitch
    
    generateModelMatrix();
    return true;
}

void Fish1::render(UnderwaterScene& scene) {
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
}
