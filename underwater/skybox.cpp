#include <glm/gtc/matrix_transform.hpp>
#include "skybox.h"
#include "underwater_scene.h"
#include "underwater_camera.h"

#include <shaders/skybox_vert_glsl.h>
#include <shaders/skybox_frag_glsl.h>

// Static resources
std::unique_ptr<ppgso::Shader> Skybox::shader;

// Cube vertices for skybox (36 vertices, 6 faces, 2 triangles each)
static float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

void Skybox::initCube() {
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    
    // Position attribute (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
}

Skybox::Skybox() {
    // Use cubemap skybox shader (procedural sky)
    if (!shader) shader = std::make_unique<ppgso::Shader>(skybox_vert_glsl, skybox_frag_glsl);
    
    // Initialize cube geometry
    initCube();
}

Skybox::~Skybox() {
    if (skyboxVAO) glDeleteVertexArrays(1, &skyboxVAO);
    if (skyboxVBO) glDeleteBuffers(1, &skyboxVBO);
}

bool Skybox::update(UnderwaterScene& scene, float dt) {
    // Skybox doesn't need model matrix - it's rendered with special technique
    return true;
}

void Skybox::render(UnderwaterScene& scene) {
    // Change depth function to LEQUAL so skybox passes at z=1.0
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    
    shader->use();
    
    shader->setUniform("ProjectionMatrix", scene.camera->projectionMatrix);
    shader->setUniform("ViewMatrix", scene.camera->viewMatrix);
    shader->setUniform("Time", scene.globalTime);
    shader->setUniform("SunDirection", scene.lightDirection);
    
    // Draw the cube
    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    // Restore default depth function
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
}
