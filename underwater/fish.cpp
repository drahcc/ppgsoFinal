#include <glm/gtc/matrix_transform.hpp>
#include "fish.h"
#include "underwater_scene.h"
#include "underwater_camera.h"

#include <shaders/underwater_vert_glsl.h>
#include <shaders/underwater_frag_glsl.h>

// Static resources
std::unique_ptr<ppgso::Mesh> Fish::mesh;
std::unique_ptr<ppgso::Texture> Fish::texture;
std::unique_ptr<ppgso::Shader> Fish::shader;

Fish::Fish() {
    // Load shared resources - underwater shader with fog
    if (!shader) shader = std::make_unique<ppgso::Shader>(underwater_vert_glsl, underwater_frag_glsl);
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("fish2/13007_Blue-Green_Reef_Chromis_v2_l3.obj");
    if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("fish2/13004_Bicolor_Blenny_v1_diff.bmp"));

    // Default scale
    scale = {0.5f, 0.5f, 0.5f};
    
    // Random initial direction
    currentYaw = static_cast<float>(rand()) / RAND_MAX * 6.28f;
    targetYaw = currentYaw;
    
    // Random time until first turn
    timeUntilNextTurn = 2.0f + static_cast<float>(rand()) / RAND_MAX * 4.0f;
    
    // Random tail phase
    tailPhase = static_cast<float>(rand()) / RAND_MAX * 6.28f;
}

bool Fish::update(UnderwaterScene& scene, float dt) {
    age += dt;
    if (lifetime > 0 && age > lifetime) {
        return false;
    }
    
    // Update tail animation
    tailPhase += swimSpeed * dt * 2.0f;
    
    // Update turn timer
    timeSinceLastTurn += dt;
    
    // Time to pick a new direction?
    if (timeSinceLastTurn >= timeUntilNextTurn) {
        timeSinceLastTurn = 0.0f;
        // Next turn in 3-7 seconds
        timeUntilNextTurn = 3.0f + static_cast<float>(rand()) / RAND_MAX * 4.0f;
        
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
    
    // Smoothly rotate towards target direction
    float yawDiff = targetYaw - currentYaw;
    // Normalize angle difference to -PI to PI
    while (yawDiff > 3.14159f) yawDiff -= 6.28318f;
    while (yawDiff < -3.14159f) yawDiff += 6.28318f;
    // Smooth turn
    currentYaw += yawDiff * turnSpeed * dt;
    
    // Calculate swim direction from current yaw (HORIZONTAL)
    swimDirection = glm::vec3(sin(currentYaw), 0.0f, cos(currentYaw));
    
    // Move forward in swim direction
    velocity = swimDirection * swimSpeed;
    position += velocity * dt;
    
    // Keep fish at reasonable depth (slight vertical variation)
    float targetY = schoolCenter.y + sin(tailPhase * 0.3f) * 1.0f;
    position.y += (targetY - position.y) * dt * 0.5f;
    
    // Keep fish in bounds
    if (position.y < -8.0f) position.y = -8.0f;
    if (position.y > -2.0f) position.y = -2.0f;
    
    // Set rotation - fish faces swimming direction
    rotation.y = currentYaw;
    rotation.z = sin(tailPhase) * 0.15f;  // Tail wag
    rotation.x = sin(tailPhase * 0.5f) * 0.05f;  // Slight pitch
    
    generateModelMatrix();
    return true;
}

void Fish::render(UnderwaterScene& scene) {
    shader->use();
    
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
    
    shader->setUniform("Texture", *texture);
    shader->setUniform("Transparency", 1.0f);
    shader->setUniform("TextureOffset", glm::vec2(0.0f));
    
    mesh->render();
}

void Fish::setTarget(glm::vec3 target) {
    targetYaw = atan2(target.x - position.x, target.z - position.z);
}

void Fish::setSpeed(float speed) {
    swimSpeed = speed;
}

void Fish::setSchool(int id, glm::vec3 center) {
    schoolId = id;
    schoolCenter = center;
}
