#include "character.h"
#include <stdio.h>

// Initialize a character
void character_init(Character* character, float x, float y, float z) {
    character->x = x;
    character->y = y;
    character->z = z;
    character->velocityY = 0.0f;
    character->isGrounded = true;
    character->isMoving = false;
    character->isAttacking = false;
    character->attackCooldown = 0.0f;
    character->state = CHARACTER_STATE_IDLE;
    
    // Initialize the animator
    character_animator_init(&character->animator);
}

// Update character state
void character_update(Character* character, float deltaTime) {
    // This is handled in updateWorld() for now
}

// Render the character
void character_render(Character* character, Shader* shader, float scale) {
    vec3 position = {character->x, character->y, character->z};
    character_animator_render(&character->animator, shader, position, scale);
}

// Clean up character resources
void character_cleanup(Character* character) {
    character_animator_cleanup(&character->animator);
} 