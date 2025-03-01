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

// Function to initialize the game world
void initWorld() {
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
}

// Function to update the game world
void updateWorld() {
    // Calculate delta time
    double currentTime = glfwGetTime();
    float deltaTime = (float)(currentTime - lastFrameTime);
    lastFrameTime = currentTime;
    
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
            printf("Secondary attack triggered!\n");
        }
    }
    
    // Determine character state
    CharacterState newState;
    
    if (isAttacking) {
        if (!player.isGrounded) {
            newState = CHARACTER_STATE_AIR_ATTACK;
            printf("Air attack triggered!\n");
        } else {
            newState = CHARACTER_STATE_ATTACK;
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
}

// Function to render the game world
void renderWorld(float aspectRatio) {
    // Calculate delta time for camera smoothing
    double currentTime = glfwGetTime();
    float deltaTime = (float)(currentTime - lastFrameTime);
    
    // Clear the screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use the shader
    shader_use(&shader);
    
    // Camera parameters based on current mode
    float cameraHeight, cameraDistance;
    vec3 cameraOffset;

    switch (currentCameraMode) {
        case CAMERA_MODE_SIDE:
            // Side-scrolling view - bring it closer
            cameraHeight = 1.0f;
            cameraDistance = 5.0f;  // Reduced from 8.0f
            cameraOffset[0] = 5.0f;  // Reduced from 8.0f
            cameraOffset[1] = cameraHeight;
            cameraOffset[2] = 0.0f;
            break;
            
        case CAMERA_MODE_TOP_DOWN:
            // Top-down view - bring it closer
            cameraHeight = 6.0f;  // Reduced from 8.0f
            cameraDistance = 0.1f;
            cameraOffset[0] = 0.0f;
            cameraOffset[1] = cameraHeight;
            cameraOffset[2] = cameraDistance;
            break;
            
        case CAMERA_MODE_FOLLOW:
        default:
            // Follow behind player - bring it closer
            cameraHeight = 1.5f;
            cameraDistance = 3.0f;  // Reduced from 5.0f
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
    vec3 cameraTarget = {
        player.x,                  // Look at player X position
        GROUND_LEVEL + 0.5f,       // Look at a fixed height slightly above ground level
        player.z                   // Look at player Z position
    };

    vec3 up = {0.0f, 1.0f, 0.0f};

    mat4 view = GLM_MAT4_IDENTITY_INIT;
    mat4 projection = GLM_MAT4_IDENTITY_INIT;
    
    glm_lookat(cameraTargetPosition, cameraTarget, up, view);
    glm_perspective(glm_rad(45.0f), aspectRatio, 0.1f, 100.0f, projection);
    
    // Set shader uniforms
    shader_set_mat4(&shader, "view", view);
    shader_set_mat4(&shader, "projection", projection);
    shader_set_vec3(&shader, "viewPos", cameraTargetPosition);
    
    // Set light properties
    vec3 lightPos = {player.x, 10.0f, player.z}; // Light follows player
    vec3 lightColor = {1.0f, 1.0f, 1.0f};
    shader_set_vec3(&shader, "lightPos", lightPos);
    shader_set_vec3(&shader, "lightColor", lightColor);
    
    // Draw grid
    drawGrid();
    
    // Use sprite shader for rendering sprites
    shader_use(&spriteShader);
    
    // Set up view and projection for sprite shader
    shader_set_mat4(&spriteShader, "view", view);
    shader_set_mat4(&spriteShader, "projection", projection);
    
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
    
    // Draw player
    vec3 playerPos = {player.x, player.y, player.z};
    character_animator_render(&player.animator, &spriteShader, playerPos, 1.0f);
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
        vertices[index++] = 0.0f;
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
    
    // Set grid color
    vec3 gridColor = {0.5f, 0.5f, 0.5f};
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
} 