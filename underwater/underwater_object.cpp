#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include "underwater_object.h"

void UnderwaterObject::generateModelMatrix() {
    // Local transform
    modelMatrix = glm::translate(glm::mat4(1.0f), position)
                * glm::orientate4(rotation)
                * glm::scale(glm::mat4(1.0f), scale);
    
    // If parent exists, multiply with parent's transform
    if (parent != nullptr) {
        modelMatrix = parent->modelMatrix * modelMatrix;
    }
}
