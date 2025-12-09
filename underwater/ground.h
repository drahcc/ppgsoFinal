#ifndef GROUND_H
#define GROUND_H

#include <ppgso/ppgso.h>
#include "underwater_object.h"

/*!
 * Ground/sand plane at the bottom of the underwater scene
 */
class Ground : public UnderwaterObject {
private:
    static std::unique_ptr<ppgso::Mesh> mesh;
    static std::unique_ptr<ppgso::Texture> texture;
    static std::unique_ptr<ppgso::Shader> shader;

public:
    Ground();

    bool update(UnderwaterScene& scene, float dt) override;
    void render(UnderwaterScene& scene) override;
};

#endif // GROUND_H
