#ifndef SPRITE_H
#define SPRITE_H

#include <glad/glad.h>
#include <stdbool.h>
#include <cglm/cglm.h>
#include "shader.h"

#define MAX_FRAMES 16

typedef struct {
    unsigned int* textureIDs;  // Array of texture IDs for animation frames
    int frameCount;            // Number of frames in the animation
    int currentFrame;          // Current frame index
    float frameDuration;       // Duration of each frame in seconds
    float timer;               // Timer for animation
    bool loop;                 // Whether to loop the animation
    
    // Add these fields to track sprite dimensions if they're not already there
    float width;               // Width of the sprite
    float height;              // Height of the sprite
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
} Sprite;

// Initialize a sprite with animation frames
void sprite_init(Sprite* sprite, const char** texturePaths, int frameCount, float frameDuration, bool loop);

// Initialize a sprite with a pre-created texture
void sprite_init_with_texture(Sprite* sprite, const char** texturePaths, int frameCount, 
                             float frameTime, bool loop, unsigned int textureID);

// Update sprite animation
void sprite_update(Sprite* sprite, float deltaTime);

// Render sprite at position with scale
void sprite_render(Sprite* sprite, Shader* shader, vec3 position, float scale);

// Render sprite with bottom at ground level
void sprite_render_grounded(Sprite* sprite, Shader* shader, vec3 position, float scale);

// Clean up sprite resources
void sprite_cleanup(Sprite* sprite);

#endif // SPRITE_H 