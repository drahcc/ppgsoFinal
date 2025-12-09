#include "bubble_generator.h"
#include "bubble.h"
#include "underwater_scene.h"
#include "underwater_camera.h"

BubbleGenerator::BubbleGenerator() {
    // Generator at bottom of scene
    position = {0, -9, 0};
}

bool BubbleGenerator::update(UnderwaterScene& scene, float dt) {
    spawnTimer += dt;
    
    // Count current bubbles (rough estimate - not all objects are bubbles)
    // In production, you'd track this more precisely
    
    // Spawn bubbles at regular intervals
    if (spawnTimer >= spawnRate) {
        spawnTimer = 0.0f;
        
        for (int i = 0; i < bubblesPerSpawn; i++) {
            auto bubble = std::make_unique<Bubble>();
            
            // Random position within spawn radius
            float angle = static_cast<float>(rand()) / RAND_MAX * 6.28f;
            float dist = static_cast<float>(rand()) / RAND_MAX * spawnRadius;
            
            bubble->position = position + glm::vec3(
                cos(angle) * dist,
                0,
                sin(angle) * dist
            );
            
            // Random size
            float size = 0.05f + static_cast<float>(rand()) / RAND_MAX * 0.15f;
            bubble->setSize(size);
            
            // Random lifetime
            float lifetime = 8.0f + static_cast<float>(rand()) / RAND_MAX * 6.0f;
            bubble->setLifetime(lifetime);
            
            scene.objects.push_back(std::move(bubble));
        }
    }
    
    return true;
}

void BubbleGenerator::render(UnderwaterScene& scene) {
    // Generator is invisible - does not render
}

void BubbleGenerator::setSpawnRate(float rate) {
    spawnRate = rate;
}

void BubbleGenerator::setBubblesPerSpawn(int count) {
    bubblesPerSpawn = count;
}

void BubbleGenerator::setSpawnRadius(float radius) {
    spawnRadius = radius;
}
