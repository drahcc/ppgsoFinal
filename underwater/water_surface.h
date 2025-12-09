#ifndef WATER_SURFACE_H
#define WATER_SURFACE_H

#include <ppgso/ppgso.h>
#include "underwater_object.h"

/*!
 * WaterSurface - Animated water surface with waves and sun reflection
 */
class WaterSurface : public UnderwaterObject {
private:
    static std::unique_ptr<ppgso::Mesh> mesh;
    static std::unique_ptr<ppgso::Texture> texture;
    static std::unique_ptr<ppgso::Shader> shader;

    float waveHeight = 0.3f;
    float waveFrequency = 0.15f;
    
    // Sun properties
    glm::vec3 sunDirection{0.5f, 0.8f, 0.3f};
    glm::vec3 sunColor{1.0f, 0.95f, 0.8f};
    glm::vec3 waterColor{0.0f, 0.4f, 0.6f};

public:
    WaterSurface();

    bool update(UnderwaterScene& scene, float dt) override;
    void render(UnderwaterScene& scene) override;
    
    void setWaveParams(float height, float frequency) {
        waveHeight = height;
        waveFrequency = frequency;
    }
    
    void setSunDirection(glm::vec3 dir) { sunDirection = glm::normalize(dir); }
    void setSunColor(glm::vec3 color) { sunColor = color; }
};

#endif // WATER_SURFACE_H
