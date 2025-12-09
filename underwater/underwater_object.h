#ifndef UNDERWATER_OBJECT_H
#define UNDERWATER_OBJECT_H

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Forward declaration
class UnderwaterScene;

/*!
 * Abstract base class for all objects in the underwater scene
 */
class UnderwaterObject {
public:
    UnderwaterObject() = default;
    UnderwaterObject(const UnderwaterObject&) = default;
    UnderwaterObject(UnderwaterObject&&) = default;
    virtual ~UnderwaterObject() = default;

    /*!
     * Update object state
     * @param scene - Reference to the scene
     * @param dt - Time delta
     * @return false to remove object from scene
     */
    virtual bool update(UnderwaterScene& scene, float dt) = 0;

    /*!
     * Render the object
     * @param scene - Reference to the scene
     */
    virtual void render(UnderwaterScene& scene) = 0;
    
    /*!
     * Check if object is translucent (for depth sorting)
     * Override in derived classes that have transparency
     */
    virtual bool isTranslucent() const { return translucent; }

    // Transform properties
    glm::vec3 position{0, 0, 0};
    glm::vec3 rotation{0, 0, 0};
    glm::vec3 scale{1, 1, 1};
    glm::mat4 modelMatrix{1.0f};
    
    // Parent object for hierarchical scene
    UnderwaterObject* parent = nullptr;
    
    // Transparency flag for depth sorting
    bool translucent = false;

protected:
    /*!
     * Generate model matrix from position, rotation, scale
     * Takes parent transform into account for hierarchical scene
     */
    void generateModelMatrix();
};

#endif // UNDERWATER_OBJECT_H
