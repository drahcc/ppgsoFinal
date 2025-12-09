#ifndef BUBBLE_H
#define BUBBLE_H

#include <ppgso/ppgso.h>
#include "underwater_object.h"

/*!
 * Bubble particle that rises to the surface
 * Part of the particle system for underwater effects
 */
class Bubble : public UnderwaterObject {
private:
    static std::unique_ptr<ppgso::Mesh> mesh;
    static std::unique_ptr<ppgso::Texture> texture;
    static std::unique_ptr<ppgso::Shader> shader;

    // Physics
    glm::vec3 velocity{0, 0, 0};
    float riseSpeed = 2.0f;
    float wobbleFreq = 3.0f;
    float wobbleAmp = 0.3f;
    float wobblePhase = 0.0f;
    
    // Lifetime
    float age = 0.0f;
    float lifetime = 10.0f;  // Seconds before bubble pops
    
    // Visual
    float transparency = 0.6f;

public:
    Bubble();

    bool update(UnderwaterScene& scene, float dt) override;
    void render(UnderwaterScene& scene) override;
    
    /*!
     * Set bubble properties
     */
    void setRiseSpeed(float speed);
    void setSize(float size);
    void setLifetime(float life);
};

#endif // BUBBLE_H
