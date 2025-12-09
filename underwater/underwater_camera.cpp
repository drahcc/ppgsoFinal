#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "underwater_camera.h"
#include "underwater_scene.h"

UnderwaterCamera::UnderwaterCamera(float fov, float ratio, float near, float far) {
    float fovRad = glm::radians(fov);
    projectionMatrix = glm::perspective(fovRad, ratio, near, far);
}

void UnderwaterCamera::update(UnderwaterScene& scene, float dt) {
    // Update animation
    if (animating && !keyframes.empty()) {
        animationTime += dt;
        interpolateKeyframes();
    }
    
    // Generate view matrix
    viewMatrix = glm::lookAt(position, target, up);
}

void UnderwaterCamera::addKeyframe(float time, glm::vec3 pos, glm::vec3 tgt) {
    keyframes.push_back({time, pos, tgt});
}

void UnderwaterCamera::resetAnimation() {
    animationTime = 0.0f;
    currentKeyframe = 0;
}

void UnderwaterCamera::interpolateKeyframes() {
    if (keyframes.size() < 2) {
        if (!keyframes.empty()) {
            position = keyframes[0].position;
            target = keyframes[0].target;
        }
        return;
    }

    // Find current keyframe segment
    size_t i = 0;
    while (i < keyframes.size() - 1 && keyframes[i + 1].time < animationTime) {
        i++;
    }

    // If past last keyframe, loop or stay at end
    if (i >= keyframes.size() - 1) {
        // Loop animation
        animationTime = 0.0f;
        i = 0;
    }

    // Get keyframes for interpolation
    const auto& kf1 = keyframes[i];
    const auto& kf2 = keyframes[i + 1];

    // Calculate interpolation factor
    float duration = kf2.time - kf1.time;
    float t = (animationTime - kf1.time) / duration;
    
    // Smooth step for nicer interpolation
    t = t * t * (3.0f - 2.0f * t);

    // Interpolate position and target
    position = glm::mix(kf1.position, kf2.position, t);
    target = glm::mix(kf1.target, kf2.target, t);
}
