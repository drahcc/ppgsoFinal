#ifndef BUBBLE_GENERATOR_H
#define BUBBLE_GENERATOR_H

#include <ppgso/ppgso.h>
#include "underwater_object.h"

/*!
 * Generator that spawns bubbles continuously
 * Does not render itself, only manages bubble creation
 */
class BubbleGenerator : public UnderwaterObject {
private:
    float spawnTimer = 0.0f;
    float spawnRate = 0.05f;  // Seconds between spawns
    int bubblesPerSpawn = 3;  // How many bubbles to spawn at once
    float spawnRadius = 30.0f; // Area where bubbles can spawn
    int maxBubbles = 5000;     // Maximum bubbles in scene

public:
    BubbleGenerator();

    bool update(UnderwaterScene& scene, float dt) override;
    void render(UnderwaterScene& scene) override;
    
    /*!
     * Set spawn parameters
     */
    void setSpawnRate(float rate);
    void setBubblesPerSpawn(int count);
    void setSpawnRadius(float radius);
};

#endif // BUBBLE_GENERATOR_H
