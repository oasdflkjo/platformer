#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <cglm/cglm.h>
#include "shader.h"

#define MAX_PROJECTILES 32

typedef struct {
    float x, y, z;         // Position
    float velocityX, velocityZ; // Velocity (2D movement)
    float rotation;        // Rotation angle in radians
    float scale;           // Scale factor
    float lifetime;        // How long the projectile lives
    float maxLifetime;     // Maximum lifetime
    bool active;           // Whether the projectile is active
    unsigned int textureID; // Texture ID for the projectile
    bool orbitMode;        // Whether the projectile is in orbit mode
    float orbitAngle;      // Angle in the orbit
    float orbitCenterX;    // X coordinate of orbit center
    float orbitCenterZ;    // Z coordinate of orbit center
    float radius;          // Circular hitbox radius
} Projectile;

// Initialize the projectile system
void projectile_system_init(void);

// Spawn a new projectile
void spawn_projectile(float x, float y, float z, float dirX, float dirZ, float speed, float lifetime);

// Update all projectiles
void update_projectiles(float deltaTime);

// Render all projectiles
void render_projectiles(Shader* shader);

// Clean up projectile resources
void projectile_system_cleanup(void);

// Add these function declarations
void set_projectile_orbit_mode(bool enabled);
void update_orbit_center(float x, float z);
bool check_projectile_collision(float x, float z, float radius);
void handle_projectile_collision(int projectileIndex);

#endif // PROJECTILE_H 