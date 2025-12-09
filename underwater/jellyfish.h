#ifndef JELLYFISH_H
#define JELLYFISH_H

#include <ppgso/ppgso.h>
#include "underwater_object.h"

/*!
 * Jellyfish with pulsing animation
 * Translucent object that floats and pulses
 */
class Jellyfish : public UnderwaterObject {
private:
    static std::unique_ptr<ppgso::Mesh> mesh;
    static std::unique_ptr<ppgso::Texture> texture;
    static std::unique_ptr<ppgso::Shader> shader;

    // Movement - jellyfish propel by contracting their bell
    glm::vec3 velocity{0, 0, 0};
    glm::vec3 horizontalDrift{0, 0, 0};  // Gentle horizontal drift
    float verticalVelocity = 0.0f;
    
    // Pulsing animation - drives movement (MORE VISIBLE)
    float pulsePhase = 0.0f;
    float pulseSpeed = 2.5f;  // Faster pulsing
    float pulseAmplitude = 0.35f;  // Much more visible contraction
    glm::vec3 baseScale{1, 1, 1};
    
    // Propulsion - stronger upward movement
    float propulsionStrength = 4.0f;  // Strong upward thrust
    float sinkRate = 1.2f;  // Sink between pulses
    float dragFactor = 0.92f;  // Water resistance
    
    // Visual
    float transparency = 0.75f;
    
    // Bounds - start low, go high
    float minY = -9.0f;   // Near ground
    float maxY = 8.0f;    // Can go quite high

public:
    Jellyfish();

    bool update(UnderwaterScene& scene, float dt) override;
    void render(UnderwaterScene& scene) override;
    
    /*!
     * Set jellyfish properties
     */
    void setDriftDirection(glm::vec3 dir);
    void setTransparency(float alpha);
};

#endif // JELLYFISH_H
