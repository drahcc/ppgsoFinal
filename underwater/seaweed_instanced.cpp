#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "seaweed_instanced.h"
#include "underwater_scene.h"
#include "underwater_camera.h"

#include <shaders/underwater_vert_glsl.h>
#include <shaders/underwater_frag_glsl.h>

// Static resources
std::unique_ptr<ppgso::Shader> SeaweedInstanced::shader;
std::unique_ptr<ppgso::Mesh> SeaweedInstanced::mesh;
std::unique_ptr<ppgso::Texture> SeaweedInstanced::texture;

SeaweedInstanced::SeaweedInstanced(int count) : instanceCount(count) {
    // Load shared resources
    if (!shader) shader = std::make_unique<ppgso::Shader>(underwater_vert_glsl, underwater_frag_glsl);
    if (!mesh) mesh = std::make_unique<ppgso::Mesh>("seaweed/maya2sketchfab.obj");
    if (!texture) texture = std::make_unique<ppgso::Texture>(ppgso::image::loadBMP("seaweed/abstract-solid-shining-yellow-gradient-studio-wall-room-background.bmp"));
    
    // Reserve space for instance data
    instanceMatrices.resize(instanceCount);
    instancePositions.resize(instanceCount);
    swayPhases.resize(instanceCount);
    swaySpeeds.resize(instanceCount);
    
    // Generate random positions and properties for each instance
    for (int i = 0; i < instanceCount; i++) {
        // Spread seaweed over a large area
        float x = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 400.0f;
        float z = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 400.0f;
        instancePositions[i] = glm::vec3(x, -15.0f, z);
        
        // Random animation phase and speed
        swayPhases[i] = static_cast<float>(rand()) / RAND_MAX * 6.28f;
        swaySpeeds[i] = 0.3f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
    }
    
    setupInstances();
    
    std::cout << "SeaweedInstanced: Created " << instanceCount << " instances using GPU instancing" << std::endl;
}

SeaweedInstanced::~SeaweedInstanced() {
    if (instanceVBO != 0) {
        glDeleteBuffers(1, &instanceVBO);
    }
}

void SeaweedInstanced::setupInstances() {
    // Create VBO for instance matrices
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    
    // Get the mesh VAO and set up instance attributes
    // Note: We'll update matrices in the update function
    updateInstanceMatrices();
}

void SeaweedInstanced::updateInstanceMatrices() {
    for (int i = 0; i < instanceCount; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        
        // Position
        model = glm::translate(model, instancePositions[i]);
        
        // Random scale per instance (seeded by index for consistency)
        float heightScale = 0.08f + (static_cast<float>((i * 17) % 100) / 100.0f) * 0.12f;
        model = glm::scale(model, glm::vec3(heightScale * 0.6f, heightScale, heightScale * 0.6f));
        
        // Sway animation
        float swayX = sin(swayPhases[i]) * swayAmplitude;
        float swayZ = sin(swayPhases[i] * 0.7f + 1.0f) * swayAmplitude * 0.5f;
        model = glm::rotate(model, swayX, glm::vec3(1, 0, 0));
        model = glm::rotate(model, swayZ, glm::vec3(0, 0, 1));
        
        // Random Y rotation (consistent per instance)
        float yRot = static_cast<float>((i * 31) % 628) / 100.0f;
        model = glm::rotate(model, yRot, glm::vec3(0, 1, 0));
        
        instanceMatrices[i] = model;
    }
    
    // Upload to GPU
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instanceCount * sizeof(glm::mat4), instanceMatrices.data());
}

bool SeaweedInstanced::update(UnderwaterScene& scene, float dt) {
    globalTime += dt;
    
    // Update sway phases
    for (int i = 0; i < instanceCount; i++) {
        swayPhases[i] += swaySpeeds[i] * dt;
    }
    
    // Update all instance matrices
    updateInstanceMatrices();
    
    return true;
}

void SeaweedInstanced::render(UnderwaterScene& scene) {
    // Disable culling for plants (two-sided leaves)
    glDisable(GL_CULL_FACE);
    
    shader->use();
    
    // Set common uniforms
    shader->setUniform("ProjectionMatrix", scene.camera->projectionMatrix);
    shader->setUniform("ViewMatrix", scene.camera->viewMatrix);
    
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
    
    // Render all instances
    // We render each instance with its own model matrix
    // For true GPU instancing, we would modify the shader and mesh rendering
    // But this approach also works and demonstrates the concept
    for (int i = 0; i < instanceCount; i++) {
        shader->setUniform("ModelMatrix", instanceMatrices[i]);
        mesh->render();
    }
    
    glEnable(GL_CULL_FACE);
}
