#ifndef SKYBOX_H
#define SKYBOX_H

#include <ppgso/ppgso.h>
#include "underwater_object.h"

/*!
 * Skybox - Proper cubemap skybox surrounding the scene
 * Uses cube geometry with procedural sky shader
 */
class Skybox : public UnderwaterObject {
private:
    static std::unique_ptr<ppgso::Shader> shader;
    
    // Cube vertex data for skybox (no texture, shader uses direction)
    GLuint skyboxVAO = 0;
    GLuint skyboxVBO = 0;
    
    void initCube();

public:
    Skybox();
    ~Skybox();

    bool update(UnderwaterScene& scene, float dt) override;
    void render(UnderwaterScene& scene) override;
};

#endif // SKYBOX_H
