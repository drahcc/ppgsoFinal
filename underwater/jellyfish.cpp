#include <glm/gtc/matrix_transform.hpp>
#include "jellyfish.h"
#include "underwater_scene.h"
#include "underwater_camera.h"

#include <shaders/underwater_vert_glsl.h>
#include <shaders/underwater_frag_glsl.h>

// Static resources
std::unique_ptr<ppgso::Mesh> Jellyfish::mesh;
std::unique_ptr<ppgso::Texture> Jellyfish::texture;
std::unique_ptr<ppgso::Shader> Jellyfish::shader;

Jellyfish::Jellyfish() {
    // Load shared resources - underwater shader with fog
    if (!shader) shader = std::make_unique<ppgso::Shader>(underwater_vert_glsl, underwater_frag_glsl);
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("jellyfish/21443_Jellyfish_V1.obj");
    if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("jellyfish/watercol_05_05_22_01.bmp"));

    // Mark as translucent for depth-sorting
    translucent = true;

    // Model is small (coords ~0-2 units), need larger scale to be visible
    baseScale = {1.5f, 1.5f, 1.5f};  // Much larger scale
    scale = baseScale;
    
    // Random pulse phase and speed for variety
    pulsePhase = static_cast<float>(rand()) / RAND_MAX * 6.28f;
    pulseSpeed = 1.2f + static_cast<float>(rand()) / RAND_MAX * 0.6f;  // 1.2-1.8
    
    // Random gentle horizontal drift (ocean currents)
    horizontalDrift = glm::vec3(
        (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.3f,
        0.0f,
        (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.3f
    );
}

bool Jellyfish::update(UnderwaterScene& scene, float dt) {
    // Update pulse animation
    pulsePhase += pulseSpeed * dt;
    
    // Pulse effect - VERY VISIBLE bell contraction/expansion
    float pulse = sin(pulsePhase);
    
    // Bell SQUEEZES when pulse is negative (contracting)
    // Bell EXPANDS when pulse is positive (relaxing)
    float bellWidth = 1.0f + pulse * pulseAmplitude;  // Gets wider/narrower
    float bellHeight = 1.0f - pulse * pulseAmplitude * 0.6f;  // Gets shorter when wider
    
    scale.x = baseScale.x * bellWidth;
    scale.y = baseScale.y * bellHeight;
    scale.z = baseScale.z * bellWidth;
    
    // PROPULSION: Strong upward thrust during contraction
    float pulseDerivative = cos(pulsePhase) * pulseSpeed;
    
    // Contraction phase = strong upward push
    if (pulseDerivative < 0) {
        verticalVelocity += propulsionStrength * dt * (-pulseDerivative) * 0.5f;
    }
    
    // Gravity - jellyfish slowly sink when not pulsing
    verticalVelocity -= sinkRate * dt;
    
    // Water drag
    verticalVelocity *= dragFactor;
    
    // Clamp velocity
    verticalVelocity = glm::clamp(verticalVelocity, -2.0f, 3.0f);
    
    // Update position - Y is UP
    position.y += verticalVelocity * dt;
    position.x += horizontalDrift.x * dt;
    position.z += horizontalDrift.z * dt;
    
    // Gentle side-to-side drifting
    position.x += sin(pulsePhase * 0.2f) * 0.01f;
    position.z += cos(pulsePhase * 0.25f) * 0.01f;
    
    // Vertical boundaries
    if (position.y > maxY) {
        position.y = maxY;
        verticalVelocity = -0.5f;
    }
    if (position.y < minY) {
        position.y = minY;
        verticalVelocity = 1.0f;
    }
    
    // Horizontal boundaries
    float boundX = 70.0f;
    float boundZ = 70.0f;
    if (abs(position.x) > boundX) {
        horizontalDrift.x = -horizontalDrift.x;
        position.x = glm::clamp(position.x, -boundX, boundX);
    }
    if (abs(position.z) > boundZ) {
        horizontalDrift.z = -horizontalDrift.z;
        position.z = glm::clamp(position.z, -boundZ, boundZ);
    }
    
    // ROTATION: Model has Z as up, we need Y as up, bell on TOP
    // Rotate +90 degrees (PI/2) around X axis to stand upright with bell on top
    rotation.x = 1.5708f + pulse * 0.1f;  // +90 degrees + gentle tilt
    rotation.z = sin(pulsePhase * 0.3f) * 0.08f;  // Gentle roll
    rotation.y = sin(pulsePhase * 0.1f) * 0.15f;  // Slow turn
    
    generateModelMatrix();
    return true;
}

void Jellyfish::render(UnderwaterScene& scene) {
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable face culling for translucent objects
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
    
    // Fog uniforms for underwater effect
    shader->setUniform("FogColor", scene.fogColor);
    shader->setUniform("FogDensity", scene.fogDensity);
    
    // Set texture
    shader->setUniform("Texture", *texture);
    shader->setUniform("Transparency", transparency);
    shader->setUniform("TextureOffset", glm::vec2(0.0f));
    
    mesh->render();
    
    // Restore state
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void Jellyfish::setDriftDirection(glm::vec3 dir) {
    horizontalDrift = glm::vec3(dir.x, 0.0f, dir.z) * 0.3f;
}

void Jellyfish::setTransparency(float alpha) {
    transparency = alpha;
}
