#include <glm/gtc/matrix_transform.hpp>
#include "fish_fin.h"
#include "underwater_scene.h"
#include "underwater_camera.h"

#include <shaders/underwater_vert_glsl.h>
#include <shaders/underwater_frag_glsl.h>

// Static resources
std::unique_ptr<ppgso::Mesh> FishFin::mesh;
std::unique_ptr<ppgso::Texture> FishFin::texture;
std::unique_ptr<ppgso::Shader> FishFin::shader;

FishFin::FishFin() {
    // Load shared resources - use fish mesh as a simple fin representation
    if (!shader) shader = std::make_unique<ppgso::Shader>(underwater_vert_glsl, underwater_frag_glsl);
    // Use the same fish mesh but scaled down as a fin
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("fish2/13007_Blue-Green_Reef_Chromis_v2_l3.obj");
    if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("fish2/13004_Bicolor_Blenny_v1_diff.bmp"));

    // Very small scale - this is a fin/sub-part
    scale = {0.15f, 0.08f, 0.15f};
    
    // Random flap phase
    flapPhase = static_cast<float>(rand()) / RAND_MAX * 6.28f;
}

bool FishFin::update(UnderwaterScene& scene, float dt) {
    // Animate the fin flapping
    flapPhase += flapSpeed * dt;
    
    // Fin rotation animation (flapping up and down)
    rotation.z = sin(flapPhase) * flapAmplitude;
    rotation.x = cos(flapPhase * 0.5f) * flapAmplitude * 0.3f;
    
    // Position is set relative to parent via local offset
    // The generateModelMatrix will use parent's transform
    position = localOffset;
    
    generateModelMatrix();
    return true;
}

void FishFin::render(UnderwaterScene& scene) {
    shader->use();
    
    shader->setUniform("ProjectionMatrix", scene.camera->projectionMatrix);
    shader->setUniform("ViewMatrix", scene.camera->viewMatrix);
    shader->setUniform("ModelMatrix", modelMatrix);
    
    // Fog uniforms
    shader->setUniform("FogColor", scene.fogColor);
    shader->setUniform("FogDensity", scene.fogDensity);
    
    // Light uniforms
    shader->setUniform("LightDirection", scene.lightDirection);
    shader->setUniform("CameraPosition", scene.camera->position);
    
    // Point light uniforms
    shader->setUniform("PointLightPos", scene.pointLightPos);
    shader->setUniform("PointLightColor", scene.pointLightColor);
    shader->setUniform("PointLightIntensity", scene.pointLightIntensity);
    
    // Spotlight uniforms
    shader->setUniform("SpotLightPos", scene.spotLightPos);
    shader->setUniform("SpotLightDir", scene.spotLightDir);
    shader->setUniform("SpotLightColor", scene.spotLightColor);
    shader->setUniform("SpotLightCutoff", scene.spotLightCutoff);
    shader->setUniform("SpotLightIntensity", scene.spotLightIntensity);
    
    shader->setUniform("Texture", *texture);
    shader->setUniform("Transparency", 1.0f);
    shader->setUniform("TextureOffset", glm::vec2(0.0f));
    
    mesh->render();
}
