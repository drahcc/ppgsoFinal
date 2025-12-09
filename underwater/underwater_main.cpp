// Underwater Scene Demo
// 
// Features:
// - 3D textured objects (fish, jellyfish, rocks, seaweed)
// - Hierarchical scene structure
// - Procedural animation (fish swimming, jellyfish pulsing, seaweed swaying)
// - Particle system (bubbles)
// - Keyframe camera animation
// - Blinn-Phong lighting with underwater fog
// - HDR rendering with tone mapping and gamma correction
// - Post-processing effects (blur, bloom, vignette)
// - GPU Instancing for 5000+ seaweed instances
//
// Controls:
// - R: Reset scene and camera animation
// - P: Pause/Resume animation
// - 1-7: Toggle post-processing effects
// - ESC: Exit

#include <iostream>
#include <map>
#include <list>
#include <cstdlib>
#include <ctime>

#include <ppgso/ppgso.h>

#include "underwater_scene.h"
#include "underwater_camera.h"
#include "underwater_object.h"
#include "ground.h"
#include "fish.h"
#include "bubble.h"
#include "bubble_generator.h"
#include "jellyfish.h"
#include "seaweed.h"
#include "seaweed_instanced.h"
#include "rock.h"
#include "fish1.h"
#include "fish_fin.h"
#include "skybox.h"
#include "water_surface.h"

#include <shaders/postprocess_vert_glsl.h>
#include <shaders/postprocess_frag_glsl.h>

const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 720;

/*!
 * Main window for the underwater scene
 */
class UnderwaterWindow : public ppgso::Window {
private:
    UnderwaterScene scene;
    bool animate = true;
    
    // Post-processing
    GLuint framebuffer = 0;
    GLuint textureColorbuffer = 0;
    GLuint rbo = 0;
    std::unique_ptr<ppgso::Shader> postProcessShader;
    GLuint quadVAO = 0, quadVBO = 0;
    int postProcessEffect = 7;  // Default: underwater distortion
    float globalTime = 0.0f;
    
    void setupFramebuffer() {
        // Create framebuffer
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        
        // Create color texture attachment
        glGenTextures(1, &textureColorbuffer);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
        
        // Create renderbuffer for depth and stencil
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR: Framebuffer is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // Setup screen quad for post-processing
        float quadVertices[] = {
            // positions   // texcoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
            
            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };
        
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindVertexArray(0);
        
        // Load post-process shader
        postProcessShader = std::make_unique<ppgso::Shader>(postprocess_vert_glsl, postprocess_frag_glsl);
        
        std::cout << "Post-processing framebuffer initialized" << std::endl;
    }

    /*!
     * Initialize the underwater scene
     */
    void initScene() {
        scene.objects.clear();

        // Create camera with keyframe animation
        auto camera = std::make_unique<UnderwaterCamera>(60.0f, (float)WIDTH / HEIGHT, 0.1f, 500.0f);
        
        // Setup camera animation - Cinematic underwater dive sequence
        // 
        // PHASE 1: SKYBOX VIEW - Looking UP at the sky, sun, clouds (NO underwater visible)
        // PHASE 2: DIVING DOWN - Camera descends, looking down at water/floor from FAR
        // PHASE 3: APPROACHING - Floor and objects become visible from distance  
        // PHASE 4: EXPLORATION - Camera circles around underwater objects
        //
        // Water surface is at y=0, ground is at y=-15, fish swim at y=-8 to y=-12
        
        // ========== PHASE 1: PURE SKYBOX VIEW (0-12 sec) ==========
        // Camera HIGH UP (y=60+), looking UPWARD at sky - underwater NOT visible
        camera->addKeyframe(0.0f, {0, 60, 0}, {0, 100, -50});       // Looking UP at sky
        camera->addKeyframe(4.0f, {0, 60, 0}, {50, 90, -30});       // Pan right, still sky
        camera->addKeyframe(8.0f, {0, 60, 0}, {-50, 80, -40});      // Pan left, admiring sun
        camera->addKeyframe(12.0f, {0, 60, 0}, {0, 70, -60});       // Centered, about to dive
        
        // ========== PHASE 2: START DIVING - Looking down at distant floor (12-25 sec) ==========
        camera->addKeyframe(16.0f, {0, 50, 0}, {0, 30, -80});       // Start descending, looking forward
        camera->addKeyframe(20.0f, {0, 35, 0}, {0, -15, -100});     // Looking down toward water
        camera->addKeyframe(25.0f, {0, 15, 0}, {0, -50, -80});      // Approaching water, floor FAR below
        
        // ========== PHASE 3: THROUGH WATER SURFACE (25-35 sec) ==========
        camera->addKeyframe(30.0f, {0, 5, 0}, {0, -30, -60});       // Just above water
        camera->addKeyframe(35.0f, {0, -5, 0}, {0, -20, -40});      // Just below surface, floor visible far
        
        // ========== PHASE 4: DESCENDING TO FLOOR (35-50 sec) ==========
        camera->addKeyframe(40.0f, {0, -8, 10}, {0, -15, -30});     // Descending, seeing objects appear
        camera->addKeyframe(45.0f, {0, -10, 20}, {0, -14, -20});    // Getting closer to floor
        camera->addKeyframe(50.0f, {0, -11, 30}, {0, -13, 0});      // Near the objects now
        
        // ========== PHASE 5: CIRCLING AROUND UNDERWATER OBJECTS (50-100 sec) ==========
        // Circle around the center where fish, jellyfish, seaweed are
        camera->addKeyframe(58.0f, {30, -10, 0}, {0, -12, 0});      // Right side of scene
        camera->addKeyframe(66.0f, {25, -9, -25}, {-10, -12, 0});   // Back-right corner
        camera->addKeyframe(74.0f, {0, -10, -35}, {0, -12, 15});    // Behind the scene
        camera->addKeyframe(82.0f, {-25, -9, -25}, {10, -12, 0});   // Back-left corner
        camera->addKeyframe(90.0f, {-30, -10, 0}, {0, -12, 0});     // Left side of scene
        camera->addKeyframe(98.0f, {-20, -9, 25}, {10, -12, 0});    // Front-left
        
        // ========== PHASE 6: RISING BACK UP (100-130 sec) ==========
        camera->addKeyframe(105.0f, {0, -8, 30}, {0, -10, 0});      // Back to front
        camera->addKeyframe(112.0f, {0, 0, 20}, {0, 5, -30});       // Rising through water
        camera->addKeyframe(120.0f, {0, 30, 10}, {0, 50, -50});     // Back above water
        camera->addKeyframe(130.0f, {0, 60, 0}, {0, 100, -50});     // Back to sky view
        
        scene.camera = std::move(camera);

        // Add SKYBOX first (renders in background)
        auto skybox = std::make_unique<Skybox>();
        scene.objects.push_back(std::move(skybox));

        // Add ground (sand)
        auto ground = std::make_unique<Ground>();
        scene.objects.push_back(std::move(ground));

        // Add WATER SURFACE with waves - at y=0 (sea level)
        auto waterSurface = std::make_unique<WaterSurface>();
        waterSurface->setWaveParams(0.5f, 0.08f);  // Bigger, slower waves
        waterSurface->setSunDirection(glm::vec3(0.3f, 0.9f, 0.2f));  // Sun from above-right
        waterSurface->setSunColor(glm::vec3(1.0f, 0.95f, 0.85f));    // Warm sunlight
        scene.objects.push_back(std::move(waterSurface));

        // Create FISH SCHOOLS - deeper underwater (y = -8 to -12)
        // School 1 - near the center
        // HIERARCHY DEMO: These fish have child FishFin objects attached
        glm::vec3 school1Center = {0, -10, 0};
        for (int i = 0; i < 8; i++) {
            auto fish = std::make_unique<Fish>();
            fish->position = school1Center + glm::vec3(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 10.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 3.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 10.0f
            );
            fish->scale = glm::vec3(0.4f + static_cast<float>(rand()) / RAND_MAX * 0.2f);
            fish->setSpeed(6.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f);  // Fast!
            fish->setSchool(1, school1Center);
            
            // Get raw pointer before moving fish
            Fish* fishPtr = fish.get();
            scene.objects.push_back(std::move(fish));
            
            // Add child fins to this fish (HIERARCHICAL SCENE - 2 level parent/child)
            // Left dorsal fin
            auto leftFin = std::make_unique<FishFin>();
            leftFin->parent = fishPtr;  // Set parent reference
            leftFin->setLocalOffset(glm::vec3(-0.3f, 0.15f, 0.0f));  // Left side of fish
            leftFin->setFlapSpeed(10.0f + static_cast<float>(rand()) / RAND_MAX * 4.0f);
            scene.objects.push_back(std::move(leftFin));
            
            // Right dorsal fin
            auto rightFin = std::make_unique<FishFin>();
            rightFin->parent = fishPtr;  // Set parent reference
            rightFin->setLocalOffset(glm::vec3(0.3f, 0.15f, 0.0f));   // Right side of fish
            rightFin->setFlapSpeed(10.0f + static_cast<float>(rand()) / RAND_MAX * 4.0f);
            scene.objects.push_back(std::move(rightFin));
        }
        
        // School 2 - to the left
        glm::vec3 school2Center = {-30, -9, -20};
        for (int i = 0; i < 6; i++) {
            auto fish = std::make_unique<Fish>();
            fish->position = school2Center + glm::vec3(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 8.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 8.0f
            );
            fish->scale = glm::vec3(0.5f + static_cast<float>(rand()) / RAND_MAX * 0.3f);
            fish->setSpeed(5.0f + static_cast<float>(rand()) / RAND_MAX * 3.0f);
            fish->setSchool(2, school2Center);
            scene.objects.push_back(std::move(fish));
        }
        
        // School 3 - to the right
        glm::vec3 school3Center = {25, -11, 15};
        for (int i = 0; i < 7; i++) {
            auto fish = std::make_unique<Fish>();
            fish->position = school3Center + glm::vec3(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 12.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 12.0f
            );
            fish->scale = glm::vec3(0.6f + static_cast<float>(rand()) / RAND_MAX * 0.2f);
            fish->setSpeed(7.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f);  // Fastest school
            fish->setSchool(3, school3Center);
            scene.objects.push_back(std::move(fish));
        }

        // FISH1 - Second fish type (different species) - deeper underwater
        // Fish1 School 1 - near the back
        glm::vec3 fish1School1Center = {10, -10, -40};
        for (int i = 0; i < 6; i++) {
            auto fish1 = std::make_unique<Fish1>();
            fish1->position = fish1School1Center + glm::vec3(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 12.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 4.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 12.0f
            );
            fish1->scale = glm::vec3(0.5f + static_cast<float>(rand()) / RAND_MAX * 0.3f);
            fish1->setSpeed(5.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f);
            fish1->setSchool(4, fish1School1Center);
            scene.objects.push_back(std::move(fish1));
        }
        
        // Fish1 School 2 - deeper
        glm::vec3 fish1School2Center = {-20, -9, -15};
        for (int i = 0; i < 5; i++) {
            auto fish1 = std::make_unique<Fish1>();
            fish1->position = fish1School2Center + glm::vec3(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 10.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 3.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 10.0f
            );
            fish1->scale = glm::vec3(0.6f + static_cast<float>(rand()) / RAND_MAX * 0.2f);
            fish1->setSpeed(4.5f + static_cast<float>(rand()) / RAND_MAX * 2.5f);
            fish1->setSchool(5, fish1School2Center);
            scene.objects.push_back(std::move(fish1));
        }

        // ============ INSTANCED SEAWEED - 5000+ instances using GPU instancing ============
        // This demonstrates efficient instantiation for the project requirements (2p)
        auto instancedSeaweed = std::make_unique<SeaweedInstanced>(5000);
        scene.objects.push_back(std::move(instancedSeaweed));

        // Add some individual seaweed for variety (closer to camera)
        for (int i = 0; i < 50; i++) {
            auto weed = std::make_unique<Seaweed>();
            
            // Position on seabed - close to camera path
            float x = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 60.0f;
            float z = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 60.0f;
            weed->position = glm::vec3(x, -15.0f, z);
            
            // Varied heights
            float heightScale = 0.12f + static_cast<float>(rand()) / RAND_MAX * 0.18f;
            weed->scale = glm::vec3(heightScale * 0.7f, heightScale, heightScale * 0.7f);
            
            scene.objects.push_back(std::move(weed));
        }

        // Add ROCKS scattered on the seafloor (y = -15)
        // Large rocks - scattered around
        for (int i = 0; i < 20; i++) {
            auto rock = std::make_unique<Rock>();
            float x = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 200.0f;
            float z = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 200.0f;
            float s = 0.4f + static_cast<float>(rand()) / RAND_MAX * 0.6f;
            rock->scale = glm::vec3(s, s * 0.7f, s);
            rock->position = glm::vec3(x, -14.5f + s * 0.3f, z);
            scene.objects.push_back(std::move(rock));
        }
        
        // Rock clusters - groups of rocks together
        // Cluster 1 - near camera path
        for (int i = 0; i < 8; i++) {
            auto rock = std::make_unique<Rock>();
            float x = 15.0f + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 15.0f;
            float z = 20.0f + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 15.0f;
            rock->position = glm::vec3(x, -14.6f, z);
            scene.objects.push_back(std::move(rock));
        }
        
        // Cluster 2 - left side
        for (int i = 0; i < 6; i++) {
            auto rock = std::make_unique<Rock>();
            float x = -35.0f + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 12.0f;
            float z = -10.0f + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 12.0f;
            rock->position = glm::vec3(x, -14.6f, z);
            scene.objects.push_back(std::move(rock));
        }
        
        // Cluster 3 - background
        for (int i = 0; i < 10; i++) {
            auto rock = std::make_unique<Rock>();
            float x = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 60.0f;
            float z = -50.0f + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 20.0f;
            float s = 0.5f + static_cast<float>(rand()) / RAND_MAX * 0.5f;
            rock->scale = glm::vec3(s, s * 0.6f, s);
            rock->position = glm::vec3(x, -14.5f + s * 0.3f, z);
            scene.objects.push_back(std::move(rock));
        }

        // Add JELLYFISH - floating in mid-water (y = -6 to -10)
        // Group 1 - far back left
        glm::vec3 group1Center = {-25.0f, -7.0f, -60.0f};
        for (int i = 0; i < 4; i++) {
            auto jelly = std::make_unique<Jellyfish>();
            jelly->position = group1Center + glm::vec3(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 12.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 4.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 12.0f
            );
            float sizeVar = 0.9f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
            jelly->scale = glm::vec3(sizeVar);
            jelly->setTransparency(0.7f);
            scene.objects.push_back(std::move(jelly));
        }
        
        // Group 2 - far back center (main group)
        glm::vec3 group2Center = {5.0f, -6.0f, -70.0f};
        for (int i = 0; i < 5; i++) {
            auto jelly = std::make_unique<Jellyfish>();
            jelly->position = group2Center + glm::vec3(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 18.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 5.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 15.0f
            );
            float sizeVar = 1.0f + static_cast<float>(rand()) / RAND_MAX * 0.6f;
            jelly->scale = glm::vec3(sizeVar);
            jelly->setTransparency(0.65f);
            scene.objects.push_back(std::move(jelly));
        }
        
        // Group 3 - far back right
        glm::vec3 group3Center = {30.0f, -8.0f, -55.0f};
        for (int i = 0; i < 4; i++) {
            auto jelly = std::make_unique<Jellyfish>();
            jelly->position = group3Center + glm::vec3(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 10.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 4.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 10.0f
            );
            float sizeVar = 0.8f + static_cast<float>(rand()) / RAND_MAX * 0.5f;
            jelly->scale = glm::vec3(sizeVar);
            jelly->setTransparency(0.75f);
            scene.objects.push_back(std::move(jelly));
        }

        // Add bubble generator - bubbles rise from the seabed
        auto bubbleGen = std::make_unique<BubbleGenerator>();
        bubbleGen->setSpawnRate(0.015f);
        bubbleGen->setBubblesPerSpawn(3);
        bubbleGen->setSpawnRadius(50.0f);
        scene.objects.push_back(std::move(bubbleGen));

        std::cout << "Scene initialized with " << scene.objects.size() << " objects" << std::endl;
    }

public:
    /*!
     * Construct the underwater window
     */
    UnderwaterWindow() : Window{"Underwater Scene", WIDTH, HEIGHT} {
        // Seed random number generator
        srand(static_cast<unsigned int>(time(nullptr)));
        
        glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

        // Initialize OpenGL state
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // Enable face culling
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);

        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Setup post-processing framebuffer
        setupFramebuffer();

        initScene();
        
        std::cout << "\n=== Controls ===" << std::endl;
        std::cout << "R: Reset scene" << std::endl;
        std::cout << "P: Pause/Resume" << std::endl;
        std::cout << "0: No post-processing" << std::endl;
        std::cout << "1: Grayscale filter" << std::endl;
        std::cout << "2: Blur filter" << std::endl;
        std::cout << "3: Sharpen filter" << std::endl;
        std::cout << "4: Edge detection" << std::endl;
        std::cout << "5: Bloom effect" << std::endl;
        std::cout << "6: Vignette effect" << std::endl;
        std::cout << "7: Underwater distortion (default)" << std::endl;
        std::cout << "ESC: Exit" << std::endl;
    }

    /*!
     * Handle key press
     */
    void onKey(int key, int scanCode, int action, int mods) override {
        scene.keyboard[key] = action;

        // Reset scene
        if (key == GLFW_KEY_R && action == GLFW_PRESS) {
            initScene();
        }

        // Pause/Resume
        if (key == GLFW_KEY_P && action == GLFW_PRESS) {
            animate = !animate;
        }
        
        // Post-processing effect selection
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_0) { postProcessEffect = 0; std::cout << "Post-process: None" << std::endl; }
            if (key == GLFW_KEY_1) { postProcessEffect = 1; std::cout << "Post-process: Grayscale" << std::endl; }
            if (key == GLFW_KEY_2) { postProcessEffect = 2; std::cout << "Post-process: Blur" << std::endl; }
            if (key == GLFW_KEY_3) { postProcessEffect = 3; std::cout << "Post-process: Sharpen" << std::endl; }
            if (key == GLFW_KEY_4) { postProcessEffect = 4; std::cout << "Post-process: Edge Detection" << std::endl; }
            if (key == GLFW_KEY_5) { postProcessEffect = 5; std::cout << "Post-process: Bloom" << std::endl; }
            if (key == GLFW_KEY_6) { postProcessEffect = 6; std::cout << "Post-process: Vignette" << std::endl; }
            if (key == GLFW_KEY_7) { postProcessEffect = 7; std::cout << "Post-process: Underwater Distortion" << std::endl; }
        }

        // Exit
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    /*!
     * Handle mouse movement
     */
    void onCursorPos(double cursorX, double cursorY) override {
        scene.cursor.x = cursorX;
        scene.cursor.y = cursorY;
    }

    /*!
     * Handle mouse button
     */
    void onMouseButton(int button, int action, int mods) override {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            scene.cursor.left = action == GLFW_PRESS;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            scene.cursor.right = action == GLFW_PRESS;
        }
    }

    /*!
     * Main render loop
     */
    void onIdle() override {
        // Track time
        static auto time = static_cast<float>(glfwGetTime());
        float dt = animate ? static_cast<float>(glfwGetTime()) - time : 0;
        time = static_cast<float>(glfwGetTime());
        globalTime += dt;

        // ============ PASS 1: Render scene to framebuffer ============
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);
        
        // Set underwater background color - MATCH FOG COLOR for seamless blend
        glClearColor(scene.fogColor.r, scene.fogColor.g, scene.fogColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update and render scene
        scene.update(dt);
        scene.render();
        
        // ============ PASS 2: Apply post-processing to screen ============
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        
        postProcessShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        postProcessShader->setUniform("Texture", 0);
        postProcessShader->setUniform("EffectType", postProcessEffect);
        postProcessShader->setUniform("Time", globalTime);
        
        // Render fullscreen quad
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        glEnable(GL_DEPTH_TEST);
    }
};

int main() {
    // Initialize the underwater window
    UnderwaterWindow window;

    // Main loop
    while (window.pollEvents()) {}

    return EXIT_SUCCESS;
}
