#include "sprite.h"
#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "texture.h"

// Initialize a sprite with animation frames
void sprite_init(Sprite* sprite, const char** texturePaths, int frameCount, float frameTime, bool loop) {
    // Initialize animation properties
    sprite->frameCount = frameCount;
    sprite->currentFrame = 0;
    sprite->frameDuration = frameTime;
    sprite->timer = 0.0f;
    sprite->loop = loop;
    
    // Allocate memory for texture IDs
    sprite->textureIDs = (unsigned int*)malloc(frameCount * sizeof(unsigned int));
    
    // Load textures for each frame
    for (int i = 0; i < frameCount; i++) {
        sprite->textureIDs[i] = texture_load_png(texturePaths[i]);
        printf("Loaded sprite texture %d/%d: %s (ID: %u)\n", 
               i+1, frameCount, texturePaths[i], sprite->textureIDs[i]);
    }
    
    // Create a quad for the sprite
    float vertices[] = {
        // positions          // texture coords
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,   1.0f, 1.0f,
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    // Create VAO, VBO, and EBO
    glGenVertexArrays(1, &sprite->VAO);
    glGenBuffers(1, &sprite->VBO);
    glGenBuffers(1, &sprite->EBO);
    
    glBindVertexArray(sprite->VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, sprite->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
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

// Update sprite animation with frame-rate independence
void sprite_update(Sprite* sprite, float deltaTime) {
    // Update animation timer
    sprite->timer += deltaTime;
    
    // For non-looping animations that have reached the end, don't advance further
    if (!sprite->loop && sprite->currentFrame >= sprite->frameCount - 1) {
        // Just accumulate time but don't advance frames
        return;
    }
    
    // Check if it's time to advance to the next frame
    if (sprite->timer >= sprite->frameDuration) {
        // Advance to next frame
        sprite->currentFrame++;
        
        // Handle looping or end of animation
        if (sprite->currentFrame >= sprite->frameCount) {
            if (sprite->loop) {
                sprite->currentFrame = 0;
            } else {
                // For non-looping animations, stay on the last frame
                sprite->currentFrame = sprite->frameCount - 1;
            }
        }
        
        // Reset timer but keep any excess time for smoother animation
        sprite->timer = fmodf(sprite->timer, sprite->frameDuration);
        
        // Comment out or remove this debug print
        // printf("Advanced to frame %d/%d (timer reset to %.3f)\n", 
        //        sprite->currentFrame + 1, sprite->frameCount, sprite->timer);
    }
}

// Render the sprite with proper flipping
void sprite_render(Sprite* sprite, Shader* shader, vec3 position, float scale) {
    shader_use(shader);
    
    // Debug output (you can comment this out for less console spam)
    // printf("Rendering sprite at position (%.2f, %.2f, %.2f) with scale %.2f, frame %d/%d\n", 
    //       position[0], position[1], position[2], scale, 
    //       sprite->currentFrame + 1, sprite->frameCount);
    
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sprite->textureIDs[sprite->currentFrame]);
    
    // Create model matrix
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    
    // Position the sprite
    glm_translate(model, position);
    
    // Calculate aspect ratio for proper scaling
    // If we don't have actual dimensions, use a typical aspect ratio for character sprites
    float aspectRatio = 0.75f; // Typical height:width ratio (e.g., 3:4)
    
    // Scale the sprite with proper aspect ratio
    float absScale = fabsf(scale); // Use absolute value for uniform scaling
    glm_scale(model, (vec3){absScale, absScale * aspectRatio, absScale});
    
    // If scale is negative, we want to flip horizontally
    // We'll do this by applying a rotation around the Y axis
    if (scale < 0) {
        // Rotate 180 degrees around Y axis to flip horizontally
        glm_rotate(model, glm_rad(180.0f), (vec3){0.0f, 1.0f, 0.0f});
    }
    
    // Set model matrix in shader
    shader_set_mat4(shader, "model", model);
    
    // Set texture sampler
    shader_set_int(shader, "texture1", 0);
    
    // Draw sprite
    glBindVertexArray(sprite->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    // Disable blending
    glDisable(GL_BLEND);
}

// Clean up sprite resources
void sprite_cleanup(Sprite* sprite) {
    glDeleteVertexArrays(1, &sprite->VAO);
    glDeleteBuffers(1, &sprite->VBO);
    glDeleteBuffers(1, &sprite->EBO);
    
    // Delete all textures
    for (int i = 0; i < sprite->frameCount; i++) {
        glDeleteTextures(1, &sprite->textureIDs[i]);
    }
    
    // Free the texture IDs array
    free(sprite->textureIDs);
}

// Initialize a sprite with a pre-created texture
void sprite_init_with_texture(Sprite* sprite, const char** texturePaths, int frameCount, 
                             float frameTime, bool loop, unsigned int textureID) {
    // Initialize animation properties
    sprite->frameCount = frameCount;
    sprite->currentFrame = 0;
    sprite->frameDuration = frameTime;
    sprite->timer = 0.0f;
    sprite->loop = loop;
    
    // Allocate memory for texture IDs
    sprite->textureIDs = (unsigned int*)malloc(frameCount * sizeof(unsigned int));
    
    // Use the provided texture for all frames
    for (int i = 0; i < frameCount; i++) {
        sprite->textureIDs[i] = textureID;
        printf("Using provided texture for frame %d/%d (ID: %u)\n", 
               i+1, frameCount, textureID);
    }
    
    // Create a quad for the sprite with correct texture coordinates
    float vertices[] = {
        // positions          // texture coords
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, // bottom left
         0.5f, -0.5f, 0.0f,   1.0f, 1.0f, // bottom right
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, // top right
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f  // top left
    };
    
    unsigned int indices[] = {
        0, 1, 2, // first triangle
        2, 3, 0  // second triangle
    };
    
    // Create VAO, VBO, and EBO
    glGenVertexArrays(1, &sprite->VAO);
    glGenBuffers(1, &sprite->VBO);
    glGenBuffers(1, &sprite->EBO);
    
    glBindVertexArray(sprite->VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, sprite->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
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

// Render sprite with bottom at ground level
void sprite_render_grounded(Sprite* sprite, Shader* shader, vec3 position, float scale) {
    // This is a variant of sprite_render that ensures the sprite's bottom is at ground level
    
    // Calculate the height offset to place the bottom of the sprite at the ground
    float heightOffset = 0.0f;
    
    // Calculate aspect ratio for proper scaling
    float aspectRatio = 0.75f; // Typical height:width ratio (e.g., 3:4)
    
    // For now, use a fixed offset that works well with your sprites
    // Adjust for the aspect ratio we're using
    heightOffset = 0.5f * fabsf(scale) * aspectRatio;
    
    // Create a new position with the adjusted height
    vec3 adjustedPos;
    glm_vec3_copy(position, adjustedPos);
    adjustedPos[1] += heightOffset;
    
    // Call the regular sprite_render with the adjusted position
    sprite_render(sprite, shader, adjustedPos, scale);
} 