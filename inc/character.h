#ifndef CHARACTER_H
#define CHARACTER_H

#include <stdbool.h> // Add this for bool type
#include "character_animation.h"

typedef struct {
    float x, y, z;     // Position of the character in 3D
    float velocityY;   // Vertical velocity for jumping
    bool isGrounded;   // Whether the character is on the ground
    
    // Add these fields for animation state
    CharacterAnimator animator;
    CharacterState state;
    bool isMoving;
    bool isAttacking;
    float attackCooldown;
} Character;

// Declare the player variable
extern Character player;

// Initialize a character
void character_init(Character* character, float x, float y, float z);

// Update character state
void character_update(Character* character, float deltaTime);

// Render the character
void character_render(Character* character, Shader* shader, float scale);

// Clean up character resources
void character_cleanup(Character* character);

#endif // CHARACTER_H 