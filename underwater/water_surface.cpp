#include <glm/gtc/matrix_transform.hpp>
#include "water_surface.h"
#include "underwater_scene.h"
#include "underwater_camera.h"

#include <shaders/water_vert_glsl.h>
#include <shaders/water_frag_glsl.h>

// Static resources
std::unique_ptr<ppgso::Mesh> WaterSurface::mesh;
std::unique_ptr<ppgso::Texture> WaterSurface::texture;
std::unique_ptr<ppgso::Shader> WaterSurface::shader;

WaterSurface::WaterSurface() {
    // Use water shader
    if (!shader) shader = std::make_unique<ppgso::Shader>(water_vert_glsl, water_frag_glsl);
    // Use a simple quad mesh for water surface (same as ground)
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("ground/quad.obj");
    // Use ground texture as fallback (water is mostly shader-based)
    if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("ground/ground.bmp"));

    // Mark as translucent for depth-sorting
    translucent = true;

    // Large water surface
    scale = glm::vec3(500.0f, 1.0f, 500.0f);
    
    // Position at water level (y = 0 is the surface, underwater is negative)
    position = glm::vec3(0, 0.0f, 0);
}

bool WaterSurface::update(UnderwaterScene& scene, float dt) {
    // Water surface stays fixed but follows camera horizontally for infinite effect
    position.x = scene.camera->position.x;
    position.z = scene.camera->position.z;
    
    generateModelMatrix();
    return true;
}

void WaterSurface::render(UnderwaterScene& scene) {
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    shader->use();
    
    shader->setUniform("ProjectionMatrix", scene.camera->projectionMatrix);
    shader->setUniform("ViewMatrix", scene.camera->viewMatrix);
    shader->setUniform("ModelMatrix", modelMatrix);
    
    // Wave animation uniforms
    shader->setUniform("Time", scene.globalTime);
    shader->setUniform("WaveHeight", waveHeight);
    shader->setUniform("WaveFrequency", waveFrequency);
    
    // Sun and lighting
    shader->setUniform("SunDirection", sunDirection);
    shader->setUniform("SunColor", sunColor);
    shader->setUniform("WaterColor", waterColor);
    shader->setUniform("CameraPosition", scene.camera->position);
    
    shader->setUniform("Texture", *texture);
    shader->setUniform("Transparency", 0.85f);
    
    mesh->render();
    
    glDisable(GL_BLEND);
}
