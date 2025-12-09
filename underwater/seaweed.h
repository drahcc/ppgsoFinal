#ifndef SEAWEED_H
#define SEAWEED_H

#include <ppgso/ppgso.h>
#include "underwater_object.h"

/*!
 * Seaweed that sways in the underwater current
 * Uses procedural animation for swaying motion
 */
class Seaweed : public UnderwaterObject {
private:
    static std::unique_ptr<ppgso::Mesh> mesh;
    static std::unique_ptr<ppgso::Texture> texture;
    static std::unique_ptr<ppgso::Shader> shader;

    // Animation
    float swayPhase = 0.0f;
    float swaySpeed = 1.5f;
    float swayAmplitude = 0.2f;
    glm::vec3 baseRotation{0, 0, 0};

public:
    Seaweed();

    bool update(UnderwaterScene& scene, float dt) override;
    void render(UnderwaterScene& scene) override;
};

#endif // SEAWEED_H
