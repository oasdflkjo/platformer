#include "pch.h"
#include "world.h"
#include "character.h"
#include "input.h"
#include <math.h>
#include "shader.h"
#include "model.h"
#include <cglm/cglm.h>

// Define the player variable
Character player = {0.0f, 0.5f, 0.0f};

// Shader and model variables
static Shader shader;
static Model playerModel;

// Add these global variables for the grid
static unsigned int gridVAO = 0;
static unsigned int gridVBO = 0;
static int gridVertexCount = 0;

// Add these variables to track movement direction
static float playerDirection = 0.0f; // Angle in radians (0 = facing +Z)
static vec3 lastMovement = {0.0f, 0.0f, 0.0f};
static const float MODEL_ROTATION_OFFSET = GLM_PIf - GLM_PI_2f; // 180 - 90 = 90 degrees

// Add these constants for physics
static const float GRAVITY = 0.015f;        // Reduced gravity acceleration per frame
static const float JUMP_FORCE = 0.2f;       // Increased initial jump velocity
static const float GROUND_LEVEL = 0.5f;     // Y position of the ground

// Add these variables for FPS calculation
static double lastTime = 0.0;
static int frameCount = 0;
static float fps = 0.0f;

// Function to initialize the game world
void initWorld() {
    // Initialize shader
    shader_init(&shader, "assets/shaders/basic.vert", "assets/shaders/basic.frag");
    
    // Load player model
    model_load(&playerModel, "assets/model.glb");
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Initialize player
    player.x = 0.0f;
    player.y = GROUND_LEVEL;
    player.z = 0.0f;
    player.velocityY = 0.0f;
    player.isGrounded = true;
}

// Function to update the game world
void updateWorld() {
    int count;
    const float* axes = getControllerAxes(&count);

    // Handle horizontal movement
    if (count >= 2) {
        float moveSpeed = 1.4f / 60.0f;
        float deadzone = 0.1f;
        
        // Apply movement if above deadzone
        if (fabs(axes[0]) > deadzone || fabs(axes[1]) > deadzone) {
            // Update position
            player.x += axes[0] * moveSpeed;
            player.z += axes[1] * moveSpeed;
            
            // Calculate the angle
            playerDirection = atan2f(axes[1], axes[0]);
        }
    }
    
    // Handle jumping
    if (player.isGrounded) {
        // Check for jump button press
        if (isButtonPressed(BUTTON_X)) {
            player.velocityY = JUMP_FORCE;
            player.isGrounded = false;
            printf("Jump initiated! Velocity: %f\n", player.velocityY); // Debug output
        }
    } else {
        // Apply gravity and update vertical position
        player.velocityY -= GRAVITY;
        
        // Update player's Y position - this is what makes the player jump
        float newY = player.y + player.velocityY;
        
        // Check if landed
        if (newY <= GROUND_LEVEL) {
            player.y = GROUND_LEVEL;
            player.velocityY = 0.0f;
            player.isGrounded = true;
            printf("Landed on ground\n"); // Debug output
        } else {
            // Only update if we haven't landed
            player.y = newY;
        }
    }
}

// Function to render the game world
void renderWorld(float aspectRatio) {
    // Calculate FPS
    double currentTime = glfwGetTime();
    frameCount++;
    
    // Update FPS counter once per second
    if (currentTime - lastTime >= 1.0) {
        fps = (float)frameCount / (float)(currentTime - lastTime);
        printf("FPS: %.1f\n", fps);
        
        frameCount = 0;
        lastTime = currentTime;
    }
    
    // Clear the color and depth buffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use our shader
    shader_use(&shader);
    
    // Set up view and projection matrices
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    mat4 projection = GLM_MAT4_IDENTITY_INIT;
    
    // Set up camera position - follow player in X and Z, but not in Y
    vec3 cameraOffset = {0.0f, 5.0f, 5.0f}; // Camera offset from player
    vec3 cameraPos = {
        player.x + cameraOffset[0], 
        GROUND_LEVEL + cameraOffset[1], // Fixed Y position based on ground level
        player.z + cameraOffset[2]
    };
    vec3 cameraTarget = {
        player.x, 
        GROUND_LEVEL, // Fixed Y target based on ground level
        player.z
    };
    vec3 up = {0.0f, 1.0f, 0.0f};
    
    glm_lookat(cameraPos, cameraTarget, up, view);
    glm_perspective(glm_rad(45.0f), aspectRatio, 0.1f, 100.0f, projection);
    
    // Set shader uniforms
    shader_set_mat4(&shader, "view", view);
    shader_set_mat4(&shader, "projection", projection);
    shader_set_vec3(&shader, "viewPos", cameraPos);
    
    // Set light properties
    vec3 lightPos = {0.0f, 10.0f, 0.0f};
    vec3 lightColor = {1.0f, 1.0f, 1.0f};
    shader_set_vec3(&shader, "lightPos", lightPos);
    shader_set_vec3(&shader, "lightColor", lightColor);
    
    // Draw grid
    drawGrid();
    
    // Draw player model with rotation based on movement direction
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, (vec3){player.x, player.y, player.z});
    
    // Apply rotation based on movement direction
    glm_rotate(model, -playerDirection + MODEL_ROTATION_OFFSET, (vec3){0.0f, 1.0f, 0.0f});
    
    // Apply scaling
    glm_scale(model, (vec3){0.5f, 0.5f, 0.5f});
    shader_set_mat4(&shader, "model", model);
    
    model_render(&playerModel, &shader);
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
    model_cleanup(&playerModel);
} 