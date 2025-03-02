#include "character_animation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logging.h"

// Define this module for logging
LOG_MODULE_DEFINE(__FILE__, false);

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

// Helper function to load all frames from a directory
int load_animation_frames(const char* basePath, char*** framePathsOut) {
#ifdef _WIN32
    // Windows implementation
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    int frameCount = 0;
    char** framePaths = NULL;
    char searchPath[512];
    
    // Create search path with wildcard
    sprintf(searchPath, "%s\\*.png", basePath);
    
    // First pass: count PNG files
    hFind = FindFirstFile(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        LOG("Failed to open directory: %s", basePath);
        return 0;
    }
    
    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            frameCount++;
        }
    } while (FindNextFile(hFind, &findData));
    
    FindClose(hFind);
    
    // Allocate memory for the frame paths
    framePaths = (char**)malloc(frameCount * sizeof(char*));
    if (!framePaths) {
        return 0;
    }
    
    // Second pass: store the file paths
    hFind = FindFirstFile(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        free(framePaths);
        return 0;
    }
    
    int index = 0;
    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            // Allocate memory for the full path
            framePaths[index] = (char*)malloc(strlen(basePath) + strlen(findData.cFileName) + 2);
            if (framePaths[index]) {
                sprintf(framePaths[index], "%s\\%s", basePath, findData.cFileName);
                index++;
            }
        }
    } while (FindNextFile(hFind, &findData));
    
    FindClose(hFind);
    *framePathsOut = framePaths;
    return frameCount;
#else
    // Unix implementation (unchanged)
    DIR* dir;
    struct dirent* entry;
    int frameCount = 0;
    char** framePaths = NULL;
    
    // Open the directory
    dir = opendir(basePath);
    if (!dir) {
        LOG("Failed to open directory: %s", basePath);
        return 0;
    }
    
    // First pass: count the number of PNG files
    while ((entry = readdir(dir)) != NULL) {
        // Check if the file has a .png extension
        char* ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".png") == 0) {
            frameCount++;
        }
    }
    
    // Allocate memory for the frame paths
    framePaths = (char**)malloc(frameCount * sizeof(char*));
    if (!framePaths) {
        closedir(dir);
        return 0;
    }
    
    // Reset directory position
    rewinddir(dir);
    
    // Second pass: store the file paths
    int index = 0;
    while ((entry = readdir(dir)) != NULL) {
        char* ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".png") == 0) {
            // Allocate memory for the full path
            framePaths[index] = (char*)malloc(strlen(basePath) + strlen(entry->d_name) + 2);
            if (framePaths[index]) {
                sprintf(framePaths[index], "%s/%s", basePath, entry->d_name);
                index++;
            }
        }
    }
    
    closedir(dir);
    *framePathsOut = framePaths;
    return frameCount;
#endif
}

// Add this global variable to track if an attack is in progress
static bool attackInProgress = false;

// Initialize the character animator
void character_animator_init(CharacterAnimator* animator) {
    animator->currentState = CHARACTER_STATE_IDLE;
    animator->facingRight = true;
    animator->stateTime = 0.0f;
    
    // Initialize all animations to empty/default state first
    for (int i = 0; i < CHARACTER_STATE_COUNT; i++) {
        // Initialize with a dummy texture that will be replaced
        const char* dummyPath = "dummy";
        sprite_init_with_texture(&animator->animations[i], &dummyPath, 1, 0.1f, true, 0);
    }
    
    // Base path for animations
    const char* basePath = "assets/Terrible Knight/Sprites";
    
    // Load each animation state
    char** framePaths;
    int frameCount;
    
    // Idle animation
    char idlePath[256];
    sprintf(idlePath, "%s/Idle", basePath);
    frameCount = load_animation_frames(idlePath, &framePaths);
    if (frameCount > 0) {
        sprite_init(&animator->animations[CHARACTER_STATE_IDLE], 
                   (const char**)framePaths, frameCount, 0.1f, true);
        
        // Free allocated memory
        for (int i = 0; i < frameCount; i++) {
            free(framePaths[i]);
        }
        free(framePaths);
    } else {
        LOG("Failed to load Idle animation");
    }
    
    // Run animation
    char runPath[256];
    sprintf(runPath, "%s/Run", basePath);
    frameCount = load_animation_frames(runPath, &framePaths);
    if (frameCount > 0) {
        sprite_init(&animator->animations[CHARACTER_STATE_RUN], 
                   (const char**)framePaths, frameCount, 0.08f, true);
        
        // Free allocated memory
        for (int i = 0; i < frameCount; i++) {
            free(framePaths[i]);
        }
        free(framePaths);
    } else {
        LOG("Failed to load Run animation");
    }
    
    // Jump animation
    char jumpPath[256];
    sprintf(jumpPath, "%s/Jump", basePath);
    frameCount = load_animation_frames(jumpPath, &framePaths);
    if (frameCount > 0) {
        sprite_init(&animator->animations[CHARACTER_STATE_JUMP], 
                   (const char**)framePaths, frameCount, 0.1f, false);
        
        // Free allocated memory
        for (int i = 0; i < frameCount; i++) {
            free(framePaths[i]);
        }
        free(framePaths);
    } else {
        LOG("Failed to load Jump animation");
    }
    
    // Attack animation
    char attackPath[256];
    sprintf(attackPath, "%s/SwordSlash", basePath);
    frameCount = load_animation_frames(attackPath, &framePaths);
    if (frameCount > 0) {
        LOG("Loading attack animation with %d frames", frameCount);
        sprite_init(&animator->animations[CHARACTER_STATE_ATTACK], 
                   (const char**)framePaths, frameCount, 0.2f, false);
        
        // Clone the attack animation for air attacks BEFORE freeing framePaths
        LOG("Setting up air attack animation with %d frames", frameCount);
        sprite_init(&animator->animations[CHARACTER_STATE_AIR_ATTACK], 
                   (const char**)framePaths, frameCount, 0.2f, false);
        
        // Free allocated memory
        for (int i = 0; i < frameCount; i++) {
            free(framePaths[i]);
        }
        free(framePaths);
    } else {
        LOG("Failed to load Attack animation from %s", attackPath);
        
        // Fallback: Create a simple 1-frame animation using the idle animation's first frame
        if (animator->animations[CHARACTER_STATE_IDLE].frameCount > 0) {
            unsigned int idleTextureID = animator->animations[CHARACTER_STATE_IDLE].textureIDs[0];
            const char* dummyPath = "dummy";
            sprite_init_with_texture(&animator->animations[CHARACTER_STATE_ATTACK], 
                                    &dummyPath, 1, 0.5f, false, idleTextureID);
            sprite_init_with_texture(&animator->animations[CHARACTER_STATE_AIR_ATTACK], 
                                    &dummyPath, 1, 0.5f, false, idleTextureID);
        }
    }
    
    // Load other animations as needed...
    
    // After loading all animations
    LOG("Animation states loaded:");
    for (int i = 0; i < CHARACTER_STATE_COUNT; i++) {
        LOG("State %d: %d frames, loop: %s", 
               i, animator->animations[i].frameCount, 
               animator->animations[i].loop ? "yes" : "no");
    }
}

// Update the animator with the current state
void character_animator_update(CharacterAnimator* animator, CharacterState newState, float deltaTime) {
    // Safety check for invalid state
    if (newState < 0 || newState >= CHARACTER_STATE_COUNT) {
        LOG("Warning: Invalid character state requested: %d", newState);
        newState = CHARACTER_STATE_IDLE;
    }
    
    // Check if current animation has finished (for non-looping animations)
    Sprite* currentSprite = &animator->animations[animator->currentState];
    bool animationFinished = false;
    
    if (!currentSprite->loop && 
        currentSprite->currentFrame >= currentSprite->frameCount - 1 && 
        currentSprite->timer >= currentSprite->frameDuration) {
        // Animation has finished
        animationFinished = true;
        
        // Only log the first time an animation finishes
        static CharacterState lastFinishedState = -1;
        if (animator->currentState != lastFinishedState) {
            LOG("Animation %d finished", animator->currentState);
            lastFinishedState = animator->currentState;
        }
    }
    
    // If we're in an attack state and the animation finished, go back to idle
    if ((animator->currentState == CHARACTER_STATE_ATTACK || 
         animator->currentState == CHARACTER_STATE_AIR_ATTACK) && 
        animationFinished) {
        // Force transition to idle or jump based on whether we're grounded
        newState = (newState == CHARACTER_STATE_AIR_ATTACK) ? 
                   CHARACTER_STATE_JUMP : CHARACTER_STATE_IDLE;
    }
    
    // When starting an attack animation
    if (newState == CHARACTER_STATE_ATTACK && animator->currentState != CHARACTER_STATE_ATTACK) {
        attackInProgress = true;
        LOG("Attack started!");
    }
    
    // When attack animation finishes
    if (attackInProgress && animationFinished) {
        attackInProgress = false;
        LOG("Attack completed!");
    }
    
    // Prevent state changes during attack animation
    if (attackInProgress && !animationFinished && 
        newState != CHARACTER_STATE_ATTACK && newState != CHARACTER_STATE_AIR_ATTACK) {
        // Don't allow changing state until attack animation completes
        // Only log this once when the state change is first attempted
        static CharacterState lastAttemptedState = -1;
        if (newState != lastAttemptedState) {
            LOG("Preventing state change to %d during attack", newState);
            lastAttemptedState = newState;
        }
        newState = animator->currentState;
    }
    
    // If state changed, reset state time
    if (newState != animator->currentState) {
        LOG("Animation state changing from %d to %d", animator->currentState, newState);
        
        // Safety check - make sure the new animation exists
        if (animator->animations[newState].frameCount > 0) {
            animator->currentState = newState;
            animator->stateTime = 0.0f;
            
            // Reset the animation to the first frame
            currentSprite = &animator->animations[animator->currentState];
            currentSprite->currentFrame = 0;
            currentSprite->timer = 0.0f;
            
            LOG("Animation started: %d with %d frames", 
                   newState, animator->animations[newState].frameCount);
        } else {
            LOG("Warning: Tried to switch to uninitialized animation state: %d", newState);
        }
    } else {
        animator->stateTime += deltaTime;
    }
    
    // Update the current animation
    sprite_update(&animator->animations[animator->currentState], deltaTime);
    
    // Only log attack animation frame changes, not every frame
    static int lastLoggedFrame = -1;
    if ((animator->currentState == CHARACTER_STATE_ATTACK || 
         animator->currentState == CHARACTER_STATE_AIR_ATTACK) && 
        lastLoggedFrame != animator->animations[animator->currentState].currentFrame) {
        
        lastLoggedFrame = animator->animations[animator->currentState].currentFrame;
        LOG("Attack animation frame changed to: %d/%d", 
               lastLoggedFrame + 1,
               animator->animations[animator->currentState].frameCount);
    }
}

// Render the current animation
void character_animator_render(CharacterAnimator* animator, Shader* shader, vec3 position, float scale) {
    // Flip the sprite based on facing direction
    float actualScale = animator->facingRight ? scale : -scale;
    
    // Safety check - only render if the animation has valid frames
    if (animator->animations[animator->currentState].frameCount > 0 &&
        animator->animations[animator->currentState].textureIDs != NULL) {
        // Render the current animation
        sprite_render_grounded(&animator->animations[animator->currentState], 
                              shader, position, actualScale);
    } else {
        LOG("Warning: Tried to render invalid animation state: %d", animator->currentState);
        // Fallback to idle animation
        if (animator->currentState != CHARACTER_STATE_IDLE && 
            animator->animations[CHARACTER_STATE_IDLE].frameCount > 0) {
            sprite_render_grounded(&animator->animations[CHARACTER_STATE_IDLE], 
                                  shader, position, actualScale);
        }
    }
}

// Clean up resources
void character_animator_cleanup(CharacterAnimator* animator) {
    for (int i = 0; i < CHARACTER_STATE_COUNT; i++) {
        sprite_cleanup(&animator->animations[i]);
    }
} 