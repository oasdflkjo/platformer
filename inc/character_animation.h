#ifndef CHARACTER_ANIMATION_H
#define CHARACTER_ANIMATION_H

#include "sprite.h"
#include <stdbool.h>

// Define character states
typedef enum {
    CHARACTER_STATE_IDLE,
    CHARACTER_STATE_RUN,
    CHARACTER_STATE_JUMP,
    CHARACTER_STATE_FALL,
    CHARACTER_STATE_ATTACK,
    CHARACTER_STATE_CROUCH,
    CHARACTER_STATE_HURT,
    CHARACTER_STATE_CLIMB,
    CHARACTER_STATE_AIR_ATTACK,
    CHARACTER_STATE_CROUCH_ATTACK,
    CHARACTER_STATE_ATTACK_UP,
    CHARACTER_STATE_ATTACK_SIDE,
    CHARACTER_STATE_COUNT  // Always keep this as the last item
} CharacterState;

// Animation manager structure
typedef struct {
    Sprite animations[CHARACTER_STATE_COUNT];
    CharacterState currentState;
    bool facingRight;
    float stateTime;  // Time spent in current state
} CharacterAnimator;

// Initialize the character animator
void character_animator_init(CharacterAnimator* animator);

// Update the animator with the current state
void character_animator_update(CharacterAnimator* animator, CharacterState newState, float deltaTime);

// Render the current animation
void character_animator_render(CharacterAnimator* animator, Shader* shader, vec3 position, float scale);

// Clean up resources
void character_animator_cleanup(CharacterAnimator* animator);

// Helper function to load all frames from a directory
int load_animation_frames(const char* basePath, char*** framePathsOut);

#endif // CHARACTER_ANIMATION_H 