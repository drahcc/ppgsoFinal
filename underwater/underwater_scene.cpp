#include <vector>
#include <algorithm>
#include "underwater_scene.h"
#include "underwater_object.h"
#include "underwater_camera.h"

void UnderwaterScene::update(float dt) {
    // Update global time
    globalTime += dt;
    
    // Update camera
    camera->update(*this, dt);
    
    // ============ Animate Point Light (bioluminescent glow moves around) ============
    pointLightPos.x = sin(globalTime * 0.5f) * 15.0f;
    pointLightPos.z = cos(globalTime * 0.3f) * 15.0f;
    pointLightPos.y = -8.0f + sin(globalTime * 0.7f) * 2.0f;
    // Pulsing intensity
    pointLightIntensity = 12.0f + sin(globalTime * 2.0f) * 5.0f;
    // Color shift (cyan to green to cyan)
    pointLightColor.g = 0.7f + sin(globalTime * 0.8f) * 0.3f;
    
    // ============ Animate Spotlight (follows camera like a diver's flashlight) ============
    spotLightPos = camera->position + glm::vec3(0.5f, -0.3f, 0.0f);
    spotLightDir = glm::normalize(camera->target - camera->position);
    // Slight flicker
    spotLightIntensity = 18.0f + sin(globalTime * 10.0f) * 2.0f;
    
    // Dynamically adjust fog based on camera depth
    // Above water (y > 0): minimal fog to see skybox
    // Below water (y < 0): increasing fog with depth
    float cameraY = camera->position.y;
    if (cameraY > 0) {
        // Above water - very light fog, sky blue color
        fogDensity = 0.0005f;
        fogColor = glm::vec3(0.4f, 0.6f, 0.85f);  // Sky blue
    } else {
        // Underwater - fog increases with depth
        float depth = -cameraY;
        fogDensity = 0.004f + depth * 0.0005f;  // More fog deeper
        fogDensity = glm::min(fogDensity, 0.015f);  // Cap fog density
        
        // Fog color gets darker with depth
        float depthFactor = glm::min(depth / 15.0f, 1.0f);
        fogColor = glm::mix(glm::vec3(0.0f, 0.3f, 0.5f),   // Shallow - lighter blue
                           glm::vec3(0.0f, 0.1f, 0.25f),   // Deep - dark blue
                           depthFactor);
    }

    // Update all objects, remove those that return false
    auto i = std::begin(objects);
    while (i != std::end(objects)) {
        auto obj = i->get();
        if (!obj->update(*this, dt))
            i = objects.erase(i);
        else
            ++i;
    }
}

void UnderwaterScene::render() {
    // Separate opaque and translucent objects
    std::vector<UnderwaterObject*> opaqueObjects;
    std::vector<UnderwaterObject*> translucentObjects;
    
    for (auto& obj : objects) {
        if (obj->isTranslucent()) {
            translucentObjects.push_back(obj.get());
        } else {
            opaqueObjects.push_back(obj.get());
        }
    }
    
    // Sort translucent objects by distance from camera (far to near)
    glm::vec3 camPos = camera->position;
    std::sort(translucentObjects.begin(), translucentObjects.end(),
        [&camPos](UnderwaterObject* a, UnderwaterObject* b) {
            float distA = glm::length(a->position - camPos);
            float distB = glm::length(b->position - camPos);
            return distA > distB;  // Far objects first
        });
    
    // Render opaque objects first (any order is fine)
    for (auto obj : opaqueObjects) {
        obj->render(*this);
    }
    
    // Render translucent objects back-to-front
    for (auto obj : translucentObjects) {
        obj->render(*this);
    }
}

