#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>
#include "shader.h"

#define MAX_ENEMIES 20

typedef struct {
    float x, y, z;         // Position
    float velocityX, velocityZ; // Velocity
    float health;          // Health points
    float radius;          // Collision radius
    bool active;           // Whether the enemy is active
    unsigned int textureID; // Texture ID for the enemy
} Enemy;

// Initialize the enemy system
void enemy_system_init(void);

// Spawn a new enemy
void spawn_enemy(float x, float y, float z);

// Update all enemies
void update_enemies(float deltaTime, float playerX, float playerZ);

// Render all enemies
void render_enemies(Shader* shader);

// Clean up enemy resources
void enemy_system_cleanup(void);

// Check if an enemy is hit by a projectile
void check_enemy_projectile_collisions(void);

// Add this function declaration
bool is_enemy_active(int index);

#endif // ENEMY_H 