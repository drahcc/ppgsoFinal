#ifndef UNDERWATER_CAMERA_H
#define UNDERWATER_CAMERA_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <ppgso/ppgso.h>

// Forward declaration
class UnderwaterScene;

/*!
 * Camera with support for animated movement through keyframes
 */
class UnderwaterCamera {
public:
    // Camera vectors
    glm::vec3 position{0, 0, 0};
    glm::vec3 target{0, 0, 0};
    glm::vec3 up{0, 1, 0};

    // Matrices
    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};

    // Keyframe for camera animation
    struct Keyframe {
        float time;
        glm::vec3 position;
        glm::vec3 target;
    };
    
    // Animation keyframes
    std::vector<Keyframe> keyframes;
    float animationTime = 0.0f;
    bool animating = true;
    int currentKeyframe = 0;

    /*!
     * Create camera with perspective projection
     * @param fov - Field of view in degrees
     * @param ratio - Aspect ratio
     * @param near - Near plane distance
     * @param far - Far plane distance
     */
    UnderwaterCamera(float fov = 60.0f, float ratio = 1.0f, float near = 0.1f, float far = 200.0f);

    /*!
     * Update camera matrices and animation
     * @param scene - Reference to the scene
     * @param dt - Time delta
     */
    void update(UnderwaterScene& scene, float dt);

    /*!
     * Add keyframe for camera animation
     */
    void addKeyframe(float time, glm::vec3 pos, glm::vec3 tgt);

    /*!
     * Reset animation to beginning
     */
    void resetAnimation();

private:
    /*!
     * Interpolate between keyframes using smooth step
     */
    void interpolateKeyframes();
};

#endif // UNDERWATER_CAMERA_H
