#ifndef FISH_FIN_H
#define FISH_FIN_H

#include <ppgso/ppgso.h>
#include "underwater_object.h"

/*!
 * Fish Fin - Child object attached to a parent Fish
 * Demonstrates hierarchical scene with 2-level parent-child relationship
 */
class FishFin : public UnderwaterObject {
private:
    static std::unique_ptr<ppgso::Mesh> mesh;
    static std::unique_ptr<ppgso::Texture> texture;
    static std::unique_ptr<ppgso::Shader> shader;

    // Animation parameters
    float flapPhase = 0.0f;
    float flapSpeed = 8.0f;
    float flapAmplitude = 0.3f;
    
    // Offset from parent
    glm::vec3 localOffset{0.0f, 0.1f, 0.0f};  // Position relative to parent

public:
    FishFin();

    bool update(UnderwaterScene& scene, float dt) override;
    void render(UnderwaterScene& scene) override;
    
    void setFlapSpeed(float speed) { flapSpeed = speed; }
    void setLocalOffset(glm::vec3 offset) { localOffset = offset; }
};

#endif // FISH_FIN_H
