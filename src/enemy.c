#include "pch.h"
#include "enemy.h"
#include "texture.h"
#include "shader.h"
#include "projectile.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// For getcwd function
#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

// Define ground level constant
#define GROUND_LEVEL 0.5f

// Enemy array
static Enemy enemies[MAX_ENEMIES];

// Enemy texture
static unsigned int enemyTextureID = 0;

// Enemy rendering data
static unsigned int enemyVAO = 0;
static unsigned int enemyVBO = 0;

// Enemy hit flash timer
static float enemyHitFlashTime[MAX_ENEMIES] = {0};

// Add these at the top of the file
static float lastPlayerX = 0.0f;
static float lastPlayerZ = 0.0f;

// Initialize the enemy system
void enemy_system_init(void) {
    // Initialize all enemies as inactive
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }
    
    // Load the enemy texture - use the Fire Skull sprite instead of slime
    enemyTextureID = texture_load_png("assets/Fire-Skull-Files/Sprites/Fire/frame1.png");
    
    if (enemyTextureID == 0) {
        printf("ERROR: Failed to load enemy texture from 'assets/Fire-Skull-Files/Sprites/Fire/frame1.png'!\n");
        printf("Current working directory: ");
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            printf("Unable to get current working directory\n");
        }
    }
    
    // Create a simple quad for the enemy
    float vertices[] = {
        // Position (XYZ), TexCoord (UV)
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f,  // Bottom left - flip Y coordinate
         0.5f, -0.5f, 0.0f,  1.0f, 1.0f,  // Bottom right - flip Y coordinate
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f,  // Top right - flip Y coordinate
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f,  // Top right - flip Y coordinate
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f,  // Top left - flip Y coordinate
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f   // Bottom left - flip Y coordinate
    };
    
    // Create VAO and VBO
    glGenVertexArrays(1, &enemyVAO);
    glGenBuffers(1, &enemyVBO);
    
    glBindVertexArray(enemyVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, enemyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Spawn a new enemy
void spawn_enemy(float x, float y, float z) {
    // Find an inactive enemy
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            // Initialize the enemy
            enemies[i].x = x;
            enemies[i].y = y + 0.5f;  // Float above the ground
            enemies[i].z = z;
            enemies[i].velocityX = 0.0f;
            enemies[i].velocityZ = 0.0f;
            enemies[i].health = 50.0f;  // Reduced from 100 to 50 for faster kills
            enemies[i].radius = 0.3f;
            enemies[i].active = true;
            enemies[i].textureID = enemyTextureID;
            
            printf("Spawned enemy %d at (%.2f, %.2f, %.2f)\n", i, x, enemies[i].y, z);
            
            return;
        }
    }
    
    // If we get here, all enemies are active
    printf("Warning: No free enemies available!\n");
}

// Update all enemies
void update_enemies(float deltaTime, float playerX, float playerZ) {
    // Store player position for rendering
    lastPlayerX = playerX;
    lastPlayerZ = playerZ;
    
    // Debug output for player position
    static int debugCounter = 0;
    if (debugCounter++ % 60 == 0) { // Print every ~60 frames
        printf("Player position: (%.2f, %.2f, %.2f)\n", playerX, GROUND_LEVEL, playerZ);
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            // Simple AI: move towards player
            float dx = playerX - enemies[i].x;
            float dz = playerZ - enemies[i].z;
            float distance = sqrtf(dx * dx + dz * dz);
            
            // Debug output for enemy position and movement
            if (i == 0 && debugCounter % 60 == 0) { // Print for first enemy every ~60 frames
                printf("Enemy %d: pos=(%.2f, %.2f, %.2f), dist=%.2f, dx=%.2f, dz=%.2f\n", 
                       i, enemies[i].x, enemies[i].y, enemies[i].z, distance, dx, dz);
            }
            
            // Only move if not too close to player
            if (distance > 0.5f) {
                // Normalize direction
                float speed = 0.5f; // Much faster speed for more noticeable movement
                
                // Calculate velocity
                enemies[i].velocityX = dx / distance * speed;
                enemies[i].velocityZ = dz / distance * speed;
                
                // Debug output for velocity
                if (i == 0 && debugCounter % 60 == 0) {
                    printf("Enemy %d velocity: (%.4f, %.4f)\n", i, enemies[i].velocityX, enemies[i].velocityZ);
                }
                
                // Update position
                float oldX = enemies[i].x;
                float oldZ = enemies[i].z;
                
                enemies[i].x += enemies[i].velocityX * deltaTime;
                enemies[i].z += enemies[i].velocityZ * deltaTime;
                
                // Debug output for position change
                if (i == 0 && debugCounter % 60 == 0) {
                    printf("Enemy %d moved: (%.2f, %.2f) -> (%.2f, %.2f), delta=(%.4f, %.4f)\n", 
                           i, oldX, oldZ, enemies[i].x, enemies[i].z, 
                           enemies[i].x - oldX, enemies[i].z - oldZ);
                }
                
                // Add a slight bobbing up and down for floating effect
                enemies[i].y = (GROUND_LEVEL + 0.3f) + sinf(glfwGetTime() * 2.0f + i) * 0.05f;
            } else {
                // Too close to player, stop moving
                enemies[i].velocityX = 0.0f;
                enemies[i].velocityZ = 0.0f;
                
                if (i == 0 && debugCounter % 60 == 0) {
                    printf("Enemy %d too close to player, stopped moving\n", i);
                }
            }
            
            // Check if enemy is dead
            if (enemies[i].health <= 0) {
                enemies[i].active = false;
                printf("Enemy %d defeated!\n", i);
            }
        }
    }
}

// Render all enemies
void render_enemies(Shader* shader) {
    if (enemyVAO == 0) {
        printf("Error: Enemy system not initialized!\n");
        return;
    }
    
    // Count active enemies
    int activeCount = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            activeCount++;
        }
    }
    
    //if (activeCount > 0) {
    //    printf("Rendering %d active enemies\n", activeCount);
    //}
    
    // Bind the VAO
    glBindVertexArray(enemyVAO);
    
    // Enable texture and set parameters
    shader_set_bool(shader, "useTexture", true);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Render each active enemy
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            // Create model matrix
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            
            // Translate to position
            glm_translate(model, (vec3){enemies[i].x, enemies[i].y, enemies[i].z});
            
            // No rotation - keep sprites facing the same direction like player
            
            // Add a subtle pulsating scale effect
            float time = glfwGetTime();
            float pulseFactor = 1.0f + sinf(time * 2.0f + i) * 0.05f;
            
            // Scale - make fire skulls a bit larger
            glm_scale(model, (vec3){0.7f * pulseFactor, 0.7f * pulseFactor, 0.7f * pulseFactor});
            
            // Set model matrix
            shader_set_mat4(shader, "model", model);
            
            // Set color - make them bright but not too bright
            vec3 color;
            if (enemyHitFlashTime[i] > 0) {
                // White flash when hit
                color[0] = 2.0f;
                color[1] = 2.0f;
                color[2] = 2.0f;
            } else {
                // Normal color with bright red/orange tint for fire
                color[0] = 2.0f;  // Red
                color[1] = 1.0f;  // Some green to make it more orange/fire-like
                color[2] = 0.3f;  // A bit of blue
            }
            shader_set_vec3(shader, "objectColor", color);
            
            // Bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, enemies[i].textureID);
            
            // Draw the enemy
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    
    // Unbind VAO
    glBindVertexArray(0);
}

// Clean up enemy resources
void enemy_system_cleanup(void) {
    if (enemyVAO != 0) {
        glDeleteVertexArrays(1, &enemyVAO);
        enemyVAO = 0;
    }
    
    if (enemyVBO != 0) {
        glDeleteBuffers(1, &enemyVBO);
        enemyVBO = 0;
    }
}

// Check if an enemy is hit by a projectile
void check_enemy_projectile_collisions(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            // Update hit flash timer
            if (enemyHitFlashTime[i] > 0) {
                enemyHitFlashTime[i] -= 0.016f; // Assuming ~60fps
            }
            
            // Check if this enemy collides with any projectile
            if (check_projectile_collision(enemies[i].x, enemies[i].z, enemies[i].radius)) {
                // Enemy was hit by a projectile
                enemies[i].health -= 25.0f; // Reduce health
                
                // Set flash effect timer
                enemyHitFlashTime[i] = 0.2f; // Flash for 0.2 seconds
                
                printf("Enemy %d hit! Health: %.1f\n", i, enemies[i].health);
            }
        }
    }
}

// Add this function implementation
bool is_enemy_active(int index) {
    if (index >= 0 && index < MAX_ENEMIES) {
        return enemies[index].active;
    }
    return false;
}

// Optimize collision detection to avoid checking inactive enemies 