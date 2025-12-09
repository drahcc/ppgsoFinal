#ifndef UNDERWATER_SCENE_H
#define UNDERWATER_SCENE_H

#include <memory>
#include <map>
#include <list>
#include <algorithm>

#include <glm/glm.hpp>

// Forward declarations
class UnderwaterObject;
class UnderwaterCamera;

/*!
 * Underwater Scene container
 * Manages all objects, camera, lighting and rendering
 * Supports multiple light sources: directional (sun), point light, spotlight
 */
class UnderwaterScene {
public:
    /*!
     * Update all objects in the scene
     * @param dt - Time delta
     */
    void update(float dt);

    /*!
     * Render all objects in the scene
     * Handles depth-sorting for translucent objects
     */
    void render();

    // Camera object
    std::unique_ptr<UnderwaterCamera> camera;

    // All objects to be rendered in scene
    std::list<std::unique_ptr<UnderwaterObject>> objects;

    // Keyboard state
    std::map<int, int> keyboard;

    // Mouse cursor state
    struct {
        double x, y;
        bool left, right;
    } cursor;

    // ============ LIGHT SOURCE 1: Directional Light (Sun) ============
    glm::vec3 lightDirection{0.3f, 0.9f, 0.2f};  // Sun direction from above
    glm::vec3 sunColor{1.0f, 0.95f, 0.85f};      // Warm sunlight color
    glm::vec3 lightColor{1.0f, 1.0f, 0.9f};
    glm::vec3 ambientColor{0.1f, 0.15f, 0.2f};
    
    // ============ LIGHT SOURCE 2: Point Light (Bioluminescent glow) ============
    glm::vec3 pointLightPos{0.0f, -8.0f, 0.0f};      // Position in scene
    glm::vec3 pointLightColor{0.2f, 0.8f, 1.0f};     // Cyan/blue glow
    float pointLightIntensity = 8.0f;                // Light intensity (reduced for balance)
    
    // ============ LIGHT SOURCE 3: Spotlight (Diver's flashlight) ============
    glm::vec3 spotLightPos{0.0f, -5.0f, 10.0f};      // Position of spotlight
    glm::vec3 spotLightDir{0.0f, -0.3f, -1.0f};      // Direction spotlight points
    glm::vec3 spotLightColor{1.0f, 1.0f, 0.9f};      // White/yellow beam
    float spotLightCutoff = 0.85f;                    // Cosine of cutoff angle (~30 degrees)
    float spotLightIntensity = 10.0f;                 // Spotlight intensity (reduced)
    
    // Underwater fog settings
    glm::vec3 fogColor{0.0f, 0.2f, 0.4f};  // Deep blue
    float fogDensity = 0.006f;
    
    // Global time for animations
    float globalTime = 0.0f;
};

#endif // UNDERWATER_SCENE_H

