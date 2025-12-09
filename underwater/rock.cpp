#include "rock.h"
#include "underwater_scene.h"
#include "underwater_camera.h"

#include <shaders/underwater_vert_glsl.h>
#include <shaders/underwater_frag_glsl.h>

// Static resources
std::unique_ptr<ppgso::Mesh> Rock::mesh;
std::unique_ptr<ppgso::Texture> Rock::texture;
std::unique_ptr<ppgso::Shader> Rock::shader;

Rock::Rock() {
    // Load shared resources
    if (!shader) shader = std::make_unique<ppgso::Shader>(underwater_vert_glsl, underwater_frag_glsl);
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("rock/Rock1_noplane.obj");  // Without base plane
    if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("rock/Rock-Texture-Surface.bmp"));

    // Random scale variation for each rock
    float s = 0.3f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
    scale = {s, s * 0.8f, s};  // Slightly flattened
    
    // Random rotation for variety
    rotation.y = static_cast<float>(rand()) / RAND_MAX * glm::pi<float>() * 2.0f;
}

bool Rock::update(UnderwaterScene& scene, float dt) {
    generateModelMatrix();
    return true;
}

void Rock::render(UnderwaterScene& scene) {
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
    
    // Set underwater fog uniforms
    shader->setUniform("FogColor", scene.fogColor);
    shader->setUniform("FogDensity", scene.fogDensity);
    
    // Set texture
    shader->setUniform("Texture", *texture);
    shader->setUniform("Transparency", 1.0f);
    shader->setUniform("TextureOffset", glm::vec2(0.0f));
    
    mesh->render();
}
