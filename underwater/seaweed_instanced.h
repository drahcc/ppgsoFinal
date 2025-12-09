#ifndef SEAWEED_INSTANCED_H
#define SEAWEED_INSTANCED_H

#include <memory>
#include <vector>
#include <ppgso/ppgso.h>
#include <glm/glm.hpp>
#include "underwater_object.h"

/*!
 * Instanced Seaweed - Renders 5000+ seaweed instances efficiently using OpenGL instancing
 * This demonstrates efficient instantiation of 3D objects for the project requirements
 */
class SeaweedInstanced : public UnderwaterObject {
private:
    // Shared resources
    static std::unique_ptr<ppgso::Shader> shader;
    static std::unique_ptr<ppgso::Mesh> mesh;
    static std::unique_ptr<ppgso::Texture> texture;
    
    // Instance data
    std::vector<glm::mat4> instanceMatrices;
    std::vector<glm::vec3> instancePositions;
    std::vector<float> swayPhases;
    std::vector<float> swaySpeeds;
    
    GLuint instanceVBO = 0;
    int instanceCount = 0;
    
    float globalTime = 0.0f;
    float swayAmplitude = 0.08f;

public:
    SeaweedInstanced(int count = 5000);
    ~SeaweedInstanced();
    
    bool update(UnderwaterScene& scene, float dt) override;
    void render(UnderwaterScene& scene) override;
    
    void setupInstances();
    void updateInstanceMatrices();
    
    int getInstanceCount() const { return instanceCount; }
};

#endif
