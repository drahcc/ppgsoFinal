#ifndef FISH1_H
#define FISH1_H

#include <ppgso/ppgso.h>
#include "underwater_object.h"

/*!
 * Fish1 - Second type of fish with different model
 * Swims in schools with natural movement
 */
class Fish1 : public UnderwaterObject {
private:
    static std::unique_ptr<ppgso::Mesh> mesh;
    static std::unique_ptr<ppgso::Texture> texture;
    static std::unique_ptr<ppgso::Shader> shader;

    // Swimming parameters
    float speed = 3.0f;
    float turnSpeed = 1.0f;
    glm::vec3 direction{0, 0, -1};
    
    // Swimming animation
    float swimPhase = 0.0f;
    float tailSwayAmount = 0.15f;
    
    // School behavior
    int schoolId = 0;
    glm::vec3 schoolCenter{0, 0, 0};
    float schoolRadius = 20.0f;
    
    // Turn timing
    float turnTimer = 0.0f;
    float nextTurnTime = 3.0f;
    float targetYaw = 0.0f;
    float currentYaw = 0.0f;

public:
    Fish1();

    bool update(UnderwaterScene& scene, float dt) override;
    void render(UnderwaterScene& scene) override;
    
    void setSpeed(float s) { speed = s; }
    void setSchool(int id, glm::vec3 center) { schoolId = id; schoolCenter = center; }
};

#endif // FISH1_H
