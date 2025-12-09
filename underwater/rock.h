#ifndef ROCK_H
#define ROCK_H

#include <ppgso/ppgso.h>
#include "underwater_object.h"

/*!
 * Static rock decoration for the underwater scene
 */
class Rock : public UnderwaterObject {
private:
    static std::unique_ptr<ppgso::Mesh> mesh;
    static std::unique_ptr<ppgso::Texture> texture;
    static std::unique_ptr<ppgso::Shader> shader;

public:
    Rock();

    bool update(UnderwaterScene& scene, float dt) override;
    void render(UnderwaterScene& scene) override;
};

#endif // ROCK_H
