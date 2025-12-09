#ifndef FISH_H
#define FISH_H

#include <ppgso/ppgso.h>
#include "underwater_object.h"

/*!
 * Fish that swims around in the underwater scene
 * Supports schooling behavior and procedural animation
 */
class Fish : public UnderwaterObject {
private:
    static std::unique_ptr<ppgso::Mesh> mesh;
    static std::unique_ptr<ppgso::Texture> texture;
    static std::unique_ptr<ppgso::Shader> shader;

    // Movement parameters
    glm::vec3 velocity{0, 0, 0};
    glm::vec3 swimDirection{0, 0, 1};  // Current swimming direction
    float swimSpeed = 5.0f;
    float turnSpeed = 1.5f;            // How fast fish turns (slower = smoother)
    
    // Timing for direction changes
    float timeSinceLastTurn = 0.0f;
    float timeUntilNextTurn = 3.0f;    // Seconds until next turn
    float targetYaw = 0.0f;            // Target Y rotation
    float currentYaw = 0.0f;           // Current Y rotation
    
    // Schooling parameters
    int schoolId = 0;
    glm::vec3 schoolCenter{0, 0, 0};
    float schoolRadius = 15.0f;
    
    // Animation parameters
    float tailPhase = 0.0f;
    float tailSpeed = 10.0f;
    float tailAmplitude = 0.2f;
    
    // Fish properties
    float age = 0.0f;
    float lifetime = -1.0f;

public:
    Fish();

    bool update(UnderwaterScene& scene, float dt) override;
    void render(UnderwaterScene& scene) override;
    
    void setTarget(glm::vec3 target);
    void setSpeed(float speed);
    void setSchool(int id, glm::vec3 center);
};

#endif // FISH_H
