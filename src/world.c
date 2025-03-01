#include "pch.h"
#include "world.h"
#include "character.h"
#include "input.h"
#include <math.h>
#include "shader.h"
#include "model.h"
#include <cglm/cglm.h>
#include "sprite.h"
#include "texture.h"
#include "../external/stb/stb_image.h"
#include "character_animation.h"
#include "projectile.h"
#include "enemy.h"

// Add this function declaration at the top of the file, after the includes
void debugRenderingIssue(void);

// Add this global variable to store the window pointer
static GLFWwindow* window = NULL;

// Define the player variable
Character player;

// Shader variables
static Shader shader;
static Shader spriteShader;

// Add these global variables for the grid
static unsigned int gridVAO = 0;
static unsigned int gridVBO = 0;
static int gridVertexCount = 0;

// Add these variables to track movement direction
static float playerDirection = 0.0f; // Angle in radians (0 = facing +Z)
static vec3 lastMovement = {0.0f, 0.0f, 0.0f};

// Add these constants for physics
static const float GRAVITY = 0.008f;
static const float JUMP_FORCE = 0.12f;
static const float GROUND_LEVEL = 0.5f;

// Add these variables for timing
static double lastFrameTime = 0.0;

// Camera smoothing variables
static vec3 cameraTargetPosition = {0.0f, 0.0f, 0.0f};
static float cameraSmoothSpeed = 5.0f; // Adjust this to control smoothing speed

// Camera mode variables
typedef enum {
    CAMERA_MODE_FOLLOW,    // Camera follows behind player
    CAMERA_MODE_SIDE,      // Side-scrolling camera
    CAMERA_MODE_TOP_DOWN,  // Top-down camera
    CAMERA_MODE_COUNT      // Number of camera modes
} CameraMode;

static CameraMode currentCameraMode = CAMERA_MODE_FOLLOW;

// Add this to the top of the file with other constants
static const float MIN_CAMERA_SMOOTH_SPEED = 3.0f;
static const float MAX_CAMERA_SMOOTH_SPEED = 10.0f;
static const float CAMERA_DISTANCE_THRESHOLD = 5.0f;

// Add this at the beginning of updateWorld
static bool debugControllerOnce = false;

// Add this at the top of the file with other global variables
static int debugCounter = 0;

// Add this global variable
static float attackFlashTimer = 0.0f;

// Add these variables at the top of world.c for wave management
static int waveNumber = 0;
static int enemiesRemainingInWave = 0;
static float waveSpawnTimer = 0.0f;
static float waveCooldown = 0.0f;
static bool waveInProgress = false;

// Add these variables at the top of the file
static float enemySpawnTimer = 0.0f;
static const float ENEMY_SPAWN_INTERVAL = 1.0f; // Spawn enemies every 1 second

// Add these function declarations at the top
void set_projectile_orbit_mode(bool enabled);
void update_orbit_center(float x, float z);

// Function to initialize the game world
void initWorld(GLFWwindow* win) {
    // Store the window pointer
    window = win;
    
    // Initialize shaders
    shader_init(&shader, "assets/shaders/basic.vert", "assets/shaders/basic.frag");
    shader_init(&spriteShader, "assets/shaders/sprite.vert", "assets/shaders/sprite.frag");
    
    // Initialize player
    character_init(&player, 0.0f, GROUND_LEVEL, 0.0f);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Initialize timing
    lastFrameTime = glfwGetTime();
    
    // Ensure viewport covers the entire window
    // (This should be set by your main rendering loop, but let's make sure)
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    // Disable scissor test which might be causing the line
    glDisable(GL_SCISSOR_TEST);
    
    // Debug OpenGL state after initialization
    printf("After initialization:\n");
    debugRenderingIssue();
    
    // Check if scissor test is enabled and disable it if it is
    GLboolean scissorEnabled;
    glGetBooleanv(GL_SCISSOR_TEST, &scissorEnabled);
    if (scissorEnabled) {
        printf("Disabling scissor test that was enabled by default\n");
        glDisable(GL_SCISSOR_TEST);
    }
    
    // Initialize projectile system
    projectile_system_init();
    enemy_system_init();

    // Spawn a test enemy at a fixed position
    spawn_enemy(2.0f, GROUND_LEVEL, 2.0f);
    printf("Spawned test enemy at (2.0, %.2f, 2.0)\n", GROUND_LEVEL);
}

// Function to update the game world
void updateWorld() {
    // Calculate delta time
    double currentTime = glfwGetTime();
    float deltaTime = (float)(currentTime - lastFrameTime);
    lastFrameTime = currentTime;

    // Debug output for deltaTime
    static int timeDebugCounter = 0;
    if (timeDebugCounter++ % 60 == 0) {
        printf("Delta time: %.6f seconds\n", deltaTime);
    }
    
    // Get controller input
    int count;
    const float* axes = getControllerAxes(&count);
    float deadzone = 0.1f;
    float controllerMoveSpeed = 0.03f;
    
    // Determine if player is moving
    bool isMoving = false;
    if (count >= 2 && (fabs(axes[0]) > deadzone || fabs(axes[1]) > deadzone)) {
        // Update position
        player.x += axes[0] * controllerMoveSpeed;
        player.z += axes[1] * controllerMoveSpeed;
        
        // Calculate the angle
        playerDirection = atan2f(axes[1], axes[0]);
        
        // Set facing direction
        player.animator.facingRight = (axes[0] > 0);
        
        isMoving = true;
    }
    
    // Handle jumping
    bool isJumping = false;
    if (player.isGrounded) {
        // Check for jump button press - only use Cross (X) for jumping, not Square
        if (isButtonPressed(BUTTON_CROSS)) {
            player.velocityY = JUMP_FORCE;
            player.isGrounded = false;
            isJumping = true;
        }
    } else {
        // Apply gravity and update vertical position
        player.velocityY -= GRAVITY;
        
        // Update player's Y position
        float newY = player.y + player.velocityY;
        
        // Check if landed
        if (newY <= GROUND_LEVEL) {
            player.y = GROUND_LEVEL;
            player.velocityY = 0.0f;
            player.isGrounded = true;
        } else {
            // Only update if we haven't landed
            player.y = newY;
            isJumping = true;
        }
    }
    
    // Check for attack buttons
    bool isAttacking = false;

    // Update attack cooldown
    if (player.attackCooldown > 0) {
        player.attackCooldown -= deltaTime;
    }

    // Only check for attacks if cooldown is done
    if (player.attackCooldown <= 0) {
        // Primary attack (Square or Circle)
        if (isButtonPressed(BUTTON_SQUARE) || isButtonPressed(BUTTON_CIRCLE)) {
            isAttacking = true;
            player.attackCooldown = 0.5f; // Set cooldown time
            printf("Attack triggered! Button: %s\n", 
                   isButtonPressed(BUTTON_SQUARE) ? "Square" : "Circle");
        }
        // Secondary attack (Triangle)
        else if (isButtonPressed(BUTTON_TRIANGLE)) {
            isAttacking = true;
            player.attackCooldown = 0.5f; // Set cooldown time
            printf("Spinning dagger attack triggered!\n");
            
            // Enable orbit mode for projectiles
            set_projectile_orbit_mode(true);
            
            // Spawn spinning daggers around the player
            float orbitLifetime = 5.0f; // Longer lifetime for orbit mode
            float spawnHeight = player.y + 0.3f;
            
            // Spawn the orbiting daggers (direction doesn't matter in orbit mode)
            spawn_projectile(player.x, spawnHeight, player.z, 0.0f, 0.0f, 0.0f, orbitLifetime);
        }
    }
    
    // Determine character state
    CharacterState newState;
    
    if (isAttacking) {
        // Check which attack button was pressed
        if (isButtonPressed(BUTTON_TRIANGLE)) {
            // For Triangle (projectile attack), don't change animation state
            // Just keep the current state (idle, run, or jump)
            if (isJumping) {
                newState = CHARACTER_STATE_JUMP;
            } else if (isMoving) {
                newState = CHARACTER_STATE_RUN;
            } else {
                newState = CHARACTER_STATE_IDLE;
            }
            
            // We could also add a special projectile casting animation state here
            // newState = CHARACTER_STATE_CAST;
        } else {
            // For other attack buttons (sword attack), use attack animation
            if (!player.isGrounded) {
                newState = CHARACTER_STATE_AIR_ATTACK;
                printf("Air attack triggered!\n");
            } else {
                newState = CHARACTER_STATE_ATTACK;
            }
        }
    } else if (isJumping) {
        newState = CHARACTER_STATE_JUMP;
    } else if (isMoving) {
        newState = CHARACTER_STATE_RUN;
    } else {
        newState = CHARACTER_STATE_IDLE;
    }
    
    // Update character animator
    character_animator_update(&player.animator, newState, deltaTime);

    // Update projectiles
    update_projectiles(deltaTime);

    // Camera mode switching
    static bool dpadUpWasPressed = false;
    bool dpadUpIsPressed = isButtonPressed(BUTTON_DPAD_UP);

    // Check for button press (only trigger once per press)
    if (dpadUpIsPressed && !dpadUpWasPressed) {
        // Cycle through camera modes
        currentCameraMode = (currentCameraMode + 1) % CAMERA_MODE_COUNT;
    }

    // Update previous state
    dpadUpWasPressed = dpadUpIsPressed;

    // Update the orbit center to follow the player
    update_orbit_center(player.x, player.z);

    // Update enemies
    update_enemies(deltaTime, player.x, player.z);

    // Check for collisions between enemies and projectiles
    check_enemy_projectile_collisions();

    // Update wave timers
    if (waveCooldown > 0.0f) {
        waveCooldown -= deltaTime;
    }

    if (waveSpawnTimer > 0.0f) {
        waveSpawnTimer -= deltaTime;
    }

    // Check if we should start a new wave
    static bool l1WasPressed = false;
    bool l1IsPressed = isButtonPressed(BUTTON_L1);

    if (l1IsPressed && !l1WasPressed && waveCooldown <= 0.0f && !waveInProgress) {
        // Start a new wave
        waveNumber++;
        waveInProgress = true;
        
        // Calculate number of enemies based on wave number
        int baseEnemies = 5;
        int enemiesPerWave = baseEnemies + (waveNumber - 1) * 2; // Increase by 2 each wave
        enemiesRemainingInWave = enemiesPerWave;
        
        // Set spawn timer for first batch
        waveSpawnTimer = 0.5f;
        
        printf("Starting Wave %d with %d enemies!\n", waveNumber, enemiesRemainingInWave);
    }

    l1WasPressed = l1IsPressed;

    // Spawn enemies in batches during a wave
    if (waveInProgress && waveSpawnTimer <= 0.0f && enemiesRemainingInWave > 0) {
        // Determine how many enemies to spawn in this batch
        int batchSize = 3;
        if (batchSize > enemiesRemainingInWave) {
            batchSize = enemiesRemainingInWave;
        }
        
        // Spawn a batch of enemies from random edges of the grid
        for (int i = 0; i < batchSize; i++) {
            // Choose a random edge (0=top, 1=right, 2=bottom, 3=left)
            int edge = rand() % 4;
            
            float gridSize = 15.0f; // Larger grid size for spawning from farther away
            float enemyX, enemyZ;
            
            switch (edge) {
                case 0: // Top edge
                    enemyX = ((float)rand() / RAND_MAX) * (gridSize * 2) - gridSize;
                    enemyZ = -gridSize;
                    break;
                case 1: // Right edge
                    enemyX = gridSize;
                    enemyZ = ((float)rand() / RAND_MAX) * (gridSize * 2) - gridSize;
                    break;
                case 2: // Bottom edge
                    enemyX = ((float)rand() / RAND_MAX) * (gridSize * 2) - gridSize;
                    enemyZ = gridSize;
                    break;
                case 3: // Left edge
                    enemyX = -gridSize;
                    enemyZ = ((float)rand() / RAND_MAX) * (gridSize * 2) - gridSize;
                    break;
            }
            
            // Spawn the enemy
            printf("Spawning enemy at (%.2f, %.2f, %.2f), distance from player: %.2f\n", 
                   enemyX, GROUND_LEVEL, enemyZ, 
                   sqrtf((enemyX - player.x) * (enemyX - player.x) + (enemyZ - player.z) * (enemyZ - player.z)));
            spawn_enemy(enemyX, GROUND_LEVEL, enemyZ);
            
            // Decrease remaining enemies
            enemiesRemainingInWave--;
        }
        
        // Set timer for next batch
        waveSpawnTimer = 1.0f;
        
        printf("Spawned %d enemies. %d remaining in wave %d\n", 
               batchSize, enemiesRemainingInWave, waveNumber);
    }

    // Check if wave is complete
    if (waveInProgress && enemiesRemainingInWave <= 0) {
        // Count active enemies
        int activeEnemies = 0;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (is_enemy_active(i)) {
                activeEnemies++;
            }
        }
        
        if (activeEnemies == 0) {
            // Wave complete
            waveInProgress = false;
            waveCooldown = 5.0f; // 5 seconds until next wave can be started
            printf("Wave %d complete! Next wave available in %.1f seconds\n", 
                   waveNumber, waveCooldown);
        }
    }

    // Update enemy spawn timer
    enemySpawnTimer -= deltaTime;

    // Spawn enemies periodically
    if (enemySpawnTimer <= 0.0f) {
        // Reset timer
        enemySpawnTimer = ENEMY_SPAWN_INTERVAL;
        
        // Spawn 2-4 enemies at random positions around the grid edges
        int numToSpawn = 2 + rand() % 3;
        
        for (int i = 0; i < numToSpawn; i++) {
            // Choose a random edge (0=top, 1=right, 2=bottom, 3=left)
            int edge = rand() % 4;
            
            float gridSize = 15.0f; // Larger grid size for spawning from farther away
            float enemyX, enemyZ;
            
            switch (edge) {
                case 0: // Top edge
                    enemyX = ((float)rand() / RAND_MAX) * (gridSize * 2) - gridSize;
                    enemyZ = -gridSize;
                    break;
                case 1: // Right edge
                    enemyX = gridSize;
                    enemyZ = ((float)rand() / RAND_MAX) * (gridSize * 2) - gridSize;
                    break;
                case 2: // Bottom edge
                    enemyX = ((float)rand() / RAND_MAX) * (gridSize * 2) - gridSize;
                    enemyZ = gridSize;
                    break;
                case 3: // Left edge
                    enemyX = -gridSize;
                    enemyZ = ((float)rand() / RAND_MAX) * (gridSize * 2) - gridSize;
                    break;
            }
            
            // Spawn the enemy
            printf("Spawning enemy at (%.2f, %.2f, %.2f), distance from player: %.2f\n", 
                   enemyX, GROUND_LEVEL, enemyZ, 
                   sqrtf((enemyX - player.x) * (enemyX - player.x) + (enemyZ - player.z) * (enemyZ - player.z)));
            spawn_enemy(enemyX, GROUND_LEVEL, enemyZ);
        }
        
        printf("Spawned %d new enemies\n", numToSpawn);
    }
}

// Function to render the game world
void renderWorld(float aspectRatio) {
    // Calculate delta time for camera smoothing
    double currentTime = glfwGetTime();
    float deltaTime = (float)(currentTime - lastFrameTime);
    
    // Debug rendering state before clearing
    static bool debugOnce = true;
    if (debugOnce) {
        printf("Before clear:\n");
        debugRenderingIssue();
        debugOnce = false;
    }
    
    // Change back to a more pleasant background color
    glClearColor(0.1f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Get the current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    // Adjust the viewport to exclude the top row where the black line appears
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3] - 1);
    
    // Use the shader
    shader_use(&shader);
    
    // Camera parameters based on current mode
    float cameraHeight, cameraDistance;
    vec3 cameraOffset;

    switch (currentCameraMode) {
        case CAMERA_MODE_SIDE:
            // Side-scrolling view - bring it closer
            cameraHeight = 1.0f;
            cameraDistance = 5.0f;
            cameraOffset[0] = 5.0f;
            cameraOffset[1] = cameraHeight;
            cameraOffset[2] = 0.0f;
            break;
            
        case CAMERA_MODE_TOP_DOWN:
            // Top-down view - bring it closer
            cameraHeight = 6.0f;
            cameraDistance = 0.1f;
            cameraOffset[0] = 0.0f;
            cameraOffset[1] = cameraHeight;
            cameraOffset[2] = cameraDistance;
            break;
            
        case CAMERA_MODE_FOLLOW:
        default:
            // Follow behind player - adjust height slightly to avoid the black line
            cameraHeight = 1.6f; // Increased slightly
            cameraDistance = 3.0f;
            cameraOffset[0] = 0.0f;
            cameraOffset[1] = cameraHeight;
            cameraOffset[2] = cameraDistance;
            break;
    }

    // Update camera target position (smoothly follow player)
    vec3 desiredCameraPos = {
        player.x + cameraOffset[0],
        cameraOffset[1],  // Fixed height, ignores player.y
        player.z + cameraOffset[2]
    };

    // Calculate distance between current camera and desired position
    glm_vec3_sub(desiredCameraPos, cameraTargetPosition, cameraOffset);
    float distanceToTarget = glm_vec3_norm(cameraOffset);

    // Adjust smoothing speed based on distance - faster when far away, slower when close
    float adjustedSmoothSpeed = MIN_CAMERA_SMOOTH_SPEED;
    if (distanceToTarget > CAMERA_DISTANCE_THRESHOLD) {
        // Gradually increase smooth speed based on distance
        float speedFactor = (distanceToTarget - CAMERA_DISTANCE_THRESHOLD) / CAMERA_DISTANCE_THRESHOLD;
        adjustedSmoothSpeed = MIN_CAMERA_SMOOTH_SPEED + 
                             (MAX_CAMERA_SMOOTH_SPEED - MIN_CAMERA_SMOOTH_SPEED) * 
                             fminf(speedFactor, 1.0f);
    }

    // Smoothly interpolate camera position with adjusted speed
    cameraTargetPosition[0] += (desiredCameraPos[0] - cameraTargetPosition[0]) * adjustedSmoothSpeed * deltaTime;
    cameraTargetPosition[1] += (desiredCameraPos[1] - cameraTargetPosition[1]) * adjustedSmoothSpeed * deltaTime;
    cameraTargetPosition[2] += (desiredCameraPos[2] - cameraTargetPosition[2]) * adjustedSmoothSpeed * deltaTime;

    // Camera target follows player horizontally but maintains a fixed vertical focus point
    // Adjust the look-at point to be slightly higher
    vec3 cameraTarget = {
        player.x,                  // Look at player X position
        GROUND_LEVEL + 0.6f,       // Look at a fixed height slightly higher above ground level
        player.z                   // Look at player Z position
    };

    vec3 up = {0.0f, 1.0f, 0.0f};

    mat4 view = GLM_MAT4_IDENTITY_INIT;
    mat4 projection = GLM_MAT4_IDENTITY_INIT;
    
    glm_lookat(cameraTargetPosition, cameraTarget, up, view);
    
    // Adjust the field of view slightly to help with the black line issue
    glm_perspective(glm_rad(42.0f), aspectRatio, 0.1f, 100.0f, projection);
    
    // Set shader uniforms
    shader_set_mat4(&shader, "view", view);
    shader_set_mat4(&shader, "projection", projection);
    shader_set_vec3(&shader, "viewPos", cameraTargetPosition);
    
    // Set light properties
    vec3 lightPos = {player.x, 10.0f, player.z}; // Light follows player
    vec3 lightColor = {1.0f, 1.0f, 1.0f};
    shader_set_vec3(&shader, "lightPos", lightPos);
    shader_set_vec3(&shader, "lightColor", lightColor);
    
    // Draw the grid for the ground plane
    drawGrid();
    
    // Use sprite shader for rendering sprites
    shader_use(&spriteShader);
    
    // Set up view and projection for sprite shader
    shader_set_mat4(&spriteShader, "view", view);
    shader_set_mat4(&spriteShader, "projection", projection);
    
    // Add texture wrapping and filtering settings to fix the black line
    // This should be done before rendering the sprite
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Add a visual flash effect during attacks
    if (player.animator.currentState == CHARACTER_STATE_ATTACK || 
        player.animator.currentState == CHARACTER_STATE_AIR_ATTACK) {
        // Slow down time slightly during attacks for dramatic effect
        deltaTime *= 0.8f;
        
        // Add a more dramatic visual effect
        // (This would be better with a proper particle system, but this is a simple approach)
        
        // Draw a simple "slash" effect
        vec3 slashPos = {
            player.x + (player.animator.facingRight ? 0.5f : -0.5f), 
            player.y + 0.5f, 
            player.z
        };
        
        // Get the current frame progress (0.0 to 1.0)
        float frameProgress = player.animator.animations[player.animator.currentState].timer / 
                             player.animator.animations[player.animator.currentState].frameDuration;
        
        // Scale the slash based on frame progress (grows and then shrinks)
        float slashScale = 1.0f;
        if (player.animator.animations[player.animator.currentState].currentFrame < 3) {
            slashScale = 0.5f + player.animator.animations[player.animator.currentState].currentFrame * 0.5f;
        } else {
            slashScale = 2.0f - (player.animator.animations[player.animator.currentState].currentFrame - 3) * 0.3f;
        }
        
        // Draw the slash effect (this would be better with a dedicated slash sprite)
        // For now, we'll just use a colored quad
        // This would require adding a simple colored quad rendering function
    } else {
        // Reset the timer when not attacking
        attackFlashTimer = 0.0f;
        shader_set_bool(&spriteShader, "hasAttackLight", false);
    }
    
    // Draw player with a slight adjustment to position
    vec3 playerPos = {player.x, player.y + 0.01f, player.z}; // Slightly raise the player
    
    // Print sprite information
    printf("Drawing sprite at position: x=%.2f, y=%.2f, z=%.2f\n", 
           player.x, player.y, player.z);
    printf("Current animation state: %d, frame: %d\n", 
           player.animator.currentState, 
           player.animator.animations[player.animator.currentState].currentFrame);

    // Add a custom flag to the sprite shader to fix the black line
    shader_set_bool(&spriteShader, "clampTexture", true);

    // Render the sprite
    character_animator_render(&player.animator, &spriteShader, playerPos, 1.0f);
    
    // Render projectiles
    render_projectiles(&spriteShader);
    
    // Render enemies
    render_enemies(&spriteShader);
    
    // Debug after all rendering is complete
    static bool debugAfterRender = true;
    if (debugAfterRender) {
        printf("After rendering:\n");
        debugRenderingIssue();
        debugAfterRender = false;
    }

    // Restore the original viewport at the end of rendering
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

// Function to initialize the grid
void initGrid() {
    float gridSize = 20.0f;
    float gridStep = 1.0f;
    
    // Count how many vertices we need
    int lineCount = (int)((gridSize * 2) / gridStep) + 1;
    gridVertexCount = lineCount * 4; // 2 lines per grid line (X and Z), 2 vertices per line
    
    // Create vertex data
    float* vertices = (float*)malloc(gridVertexCount * 3 * sizeof(float)); // 3 floats per vertex (x,y,z)
    
    int index = 0;
    for (float i = -gridSize; i <= gridSize; i += gridStep) {
        // Lines along X axis
        vertices[index++] = i;
        vertices[index++] = 0.0f; // Set Y to 0.0f exactly for all grid lines
        vertices[index++] = -gridSize;
        
        vertices[index++] = i;
        vertices[index++] = 0.0f;
        vertices[index++] = gridSize;
        
        // Lines along Z axis
        vertices[index++] = -gridSize;
        vertices[index++] = 0.0f;
        vertices[index++] = i;
        
        vertices[index++] = gridSize;
        vertices[index++] = 0.0f;
        vertices[index++] = i;
    }
    
    // Create VAO and VBO
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    
    glBindVertexArray(gridVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertexCount * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Free memory
    free(vertices);
}

// Function to draw the grid using modern OpenGL
void drawGrid() {
    // Initialize grid if not already done
    if (gridVAO == 0) {
        initGrid();
    }
    
    // Set up model matrix for grid
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    shader_set_mat4(&shader, "model", model);
    
    // Set grid color - make it slightly darker and less prominent
    vec3 gridColor = {0.3f, 0.3f, 0.3f};
    shader_set_vec3(&shader, "objectColor", gridColor);
    shader_set_bool(&shader, "useTexture", false);
    
    // Draw grid
    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, gridVertexCount);
    glBindVertexArray(0);
}

// Function to clean up resources
void cleanupWorld() {
    character_cleanup(&player);
    projectile_system_cleanup();
    enemy_system_cleanup();
}

// Add this function at the end of the file, before cleanupWorld()
void debugRenderingIssue() {
    // Get viewport dimensions
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    // Get scissor box dimensions
    GLint scissor[4];
    glGetIntegerv(GL_SCISSOR_BOX, scissor);
    
    // Check if scissor test is enabled
    GLboolean scissorEnabled;
    glGetBooleanv(GL_SCISSOR_TEST, &scissorEnabled);
    
    // Print debugging information
    printf("=== Rendering Debug Info ===\n");
    printf("Viewport: x=%d, y=%d, width=%d, height=%d\n", 
           viewport[0], viewport[1], viewport[2], viewport[3]);
    printf("Scissor: x=%d, y=%d, width=%d, height=%d, enabled=%s\n", 
           scissor[0], scissor[1], scissor[2], scissor[3], 
           scissorEnabled ? "true" : "false");
    
    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL error: 0x%04x\n", err);
    }
    printf("===========================\n");
} 