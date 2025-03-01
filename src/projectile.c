#include "pch.h"
#include "projectile.h"
#include "texture.h"
#include <stdio.h>
#include <math.h>

// Function declarations
void create_projectile_model(void);

// Projectile array
static Projectile projectiles[MAX_PROJECTILES];

// Projectile texture
static unsigned int daggerTextureID = 0;

// Projectile rendering data
static unsigned int projectileVAO = 0;
static unsigned int projectileVBO = 0;

// Add these variables at the top of the file
static bool orbitMode = false;
static float orbitRadius = 1.2f;
static float orbitSpeed = 5.0f;
static float orbitAngle = 0.0f;

// Initialize the projectile system
void projectile_system_init(void) {
    // Initialize all projectiles as inactive
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        projectiles[i].active = false;
    }
    
    // Load the dagger texture
    daggerTextureID = texture_load_png("assets/Terrible Knight/Projectiles/dagger.png");
    
    if (daggerTextureID == 0) {
        printf("Failed to load dagger texture!\n");
    }
    
    // Create a 3D model for the projectile
    create_projectile_model();
}

// Spawn a new projectile
void spawn_projectile(float x, float y, float z, float dirX, float dirZ, float speed, float lifetime) {
    // If orbit mode is enabled, we'll spawn projectiles in a circle
    if (orbitMode) {
        // Calculate how many projectiles to spawn in the orbit
        int numProjectiles = 6;  // Reduced from 8 to 6 for a cleaner look
        float angleStep = 2.0f * M_PI / numProjectiles;
        
        for (int i = 0; i < numProjectiles; i++) {
            float angle = i * angleStep;
            float orbitX = cosf(angle) * orbitRadius;
            float orbitZ = sinf(angle) * orbitRadius;
            
            // Find an inactive projectile
            for (int j = 0; j < MAX_PROJECTILES; j++) {
                if (!projectiles[j].active) {
                    // Initialize the projectile in orbit mode
                    projectiles[j].x = x + orbitX;
                    projectiles[j].y = y;
                    projectiles[j].z = z + orbitZ;
                    projectiles[j].velocityX = 0.0f;
                    projectiles[j].velocityZ = 0.0f;
                    projectiles[j].rotation = angle;
                    projectiles[j].scale = 0.3f;  // Increased from 0.25f to 0.3f for better visibility
                    projectiles[j].lifetime = lifetime;
                    projectiles[j].maxLifetime = lifetime;
                    projectiles[j].active = true;
                    projectiles[j].textureID = daggerTextureID;
                    projectiles[j].orbitAngle = angle;
                    projectiles[j].orbitMode = true;
                    projectiles[j].orbitCenterX = x;
                    projectiles[j].orbitCenterZ = z;
                    projectiles[j].radius = 0.2f; // Set hitbox radius for orbit projectiles
                    
                    break;
                }
            }
        }
    } else {
        // Original projectile spawning code for non-orbit mode
        // Find an inactive projectile
        for (int i = 0; i < MAX_PROJECTILES; i++) {
            if (!projectiles[i].active) {
                // Initialize the projectile
                projectiles[i].x = x;
                projectiles[i].y = y;
                projectiles[i].z = z;
                projectiles[i].velocityX = dirX * speed;
                projectiles[i].velocityZ = dirZ * speed;
                projectiles[i].rotation = atan2f(dirZ, dirX);
                projectiles[i].scale = 0.3f;
                projectiles[i].lifetime = lifetime;
                projectiles[i].maxLifetime = lifetime;
                projectiles[i].active = true;
                projectiles[i].textureID = daggerTextureID;
                projectiles[i].orbitMode = false;
                projectiles[i].radius = 0.2f; // Set hitbox radius for regular projectiles
                
                return;
            }
        }
    }
}

// Update all projectiles
void update_projectiles(float deltaTime) {
    // Update global orbit angle
    orbitAngle += orbitSpeed * deltaTime;
    if (orbitAngle > 2.0f * M_PI) {
        orbitAngle -= 2.0f * M_PI;
    }
    
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].active) {
            if (projectiles[i].orbitMode) {
                // Update orbit position
                projectiles[i].orbitAngle += orbitSpeed * deltaTime;
                if (projectiles[i].orbitAngle > 2.0f * M_PI) {
                    projectiles[i].orbitAngle -= 2.0f * M_PI;
                }
                
                // Calculate new position based on orbit
                projectiles[i].x = projectiles[i].orbitCenterX + cosf(projectiles[i].orbitAngle) * orbitRadius;
                projectiles[i].z = projectiles[i].orbitCenterZ + sinf(projectiles[i].orbitAngle) * orbitRadius;
                
                // Make projectiles spin on their own axis too
                projectiles[i].rotation += 10.0f * deltaTime;
            } else {
                // Regular projectile movement
                projectiles[i].x += projectiles[i].velocityX * deltaTime;
                projectiles[i].z += projectiles[i].velocityZ * deltaTime;
            }
            
            // Update lifetime
            projectiles[i].lifetime -= deltaTime;
            
            // Deactivate if lifetime is over
            if (projectiles[i].lifetime <= 0) {
                projectiles[i].active = false;
            }
        }
    }
}

// Render all projectiles
void render_projectiles(Shader* shader) {
    if (projectileVAO == 0) {
        printf("Error: Projectile system not initialized!\n");
        return;
    }
    
    // Bind the VAO
    glBindVertexArray(projectileVAO);
    
    // Enable texture and set parameters
    shader_set_bool(shader, "useTexture", true);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Render each active projectile
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].active) {
            // Create model matrix
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            
            // Translate to position
            glm_translate(model, (vec3){projectiles[i].x, projectiles[i].y, projectiles[i].z});
            
            // Rotate based on projectile type
            if (projectiles[i].orbitMode) {
                // For orbit mode, rotate based on orbit angle
                glm_rotate(model, projectiles[i].rotation, (vec3){0.0f, 1.0f, 0.0f});
            } else {
                // For regular projectiles, rotate based on direction
                if (projectiles[i].velocityX > 0.01f) {
                    // Right - no rotation needed
                } 
                else if (projectiles[i].velocityX < -0.01f) {
                    // Left - rotate 180 degrees
                    glm_rotate(model, M_PI, (vec3){0.0f, 1.0f, 0.0f});
                }
                else if (projectiles[i].velocityZ > 0.01f) {
                    // Up - rotate 270 degrees
                    glm_rotate(model, -M_PI_2, (vec3){0.0f, 1.0f, 0.0f});
                }
                else if (projectiles[i].velocityZ < -0.01f) {
                    // Down - rotate 90 degrees
                    glm_rotate(model, M_PI_2, (vec3){0.0f, 1.0f, 0.0f});
                }
            }
            
            // Scale
            glm_scale(model, (vec3){projectiles[i].scale, projectiles[i].scale, projectiles[i].scale});
            
            // Set model matrix
            shader_set_mat4(shader, "model", model);
            
            // Set color (white for normal rendering)
            vec3 color = {1.0f, 1.0f, 1.0f};
            shader_set_vec3(shader, "objectColor", color);
            
            // Bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, projectiles[i].textureID);
            
            // Draw the projectile (simple quad)
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    
    // Unbind VAO
    glBindVertexArray(0);
}

// Clean up projectile resources
void projectile_system_cleanup(void) {
    if (projectileVAO != 0) {
        glDeleteVertexArrays(1, &projectileVAO);
        projectileVAO = 0;
    }
    
    if (projectileVBO != 0) {
        glDeleteBuffers(1, &projectileVBO);
        projectileVBO = 0;
    }
}

// Simplify the create_projectile_model function to just create a simple quad
void create_projectile_model(void) {
    // Create a simple quad for the projectile
    float vertices[] = {
        // Position (XYZ), TexCoord (UV)
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f
    };
    
    // Create VAO and VBO
    glGenVertexArrays(1, &projectileVAO);
    glGenBuffers(1, &projectileVBO);
    
    glBindVertexArray(projectileVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, projectileVBO);
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

// Function to set orbit mode
void set_projectile_orbit_mode(bool enabled) {
    orbitMode = enabled;
}

// Function to update orbit center position
void update_orbit_center(float x, float z) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].active && projectiles[i].orbitMode) {
            projectiles[i].orbitCenterX = x;
            projectiles[i].orbitCenterZ = z;
        }
    }
}

// Check if a point (enemy) collides with any active projectile
bool check_projectile_collision(float x, float z, float radius) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].active) {
            // Calculate distance between projectile and point
            float dx = projectiles[i].x - x;
            float dz = projectiles[i].z - z;
            float distance = sqrtf(dx * dx + dz * dz);
            
            // Check if distance is less than sum of radii
            if (distance < (projectiles[i].radius + radius)) {
                // Collision detected
                handle_projectile_collision(i);
                return true;
            }
        }
    }
    
    // No collision
    return false;
}

// Handle what happens when a projectile collides with something
void handle_projectile_collision(int projectileIndex) {
    // For regular projectiles, deactivate them on collision
    if (!projectiles[projectileIndex].orbitMode) {
        projectiles[projectileIndex].active = false;
    }
    
    // For orbit projectiles, we don't deactivate them
    // They continue to orbit and can hit multiple enemies
    
    // You could add visual effects here, like a small flash or particle effect
    printf("Projectile %d hit something!\n", projectileIndex);
} 