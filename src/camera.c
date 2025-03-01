#include "pch.h" // Use the precompiled header
#include "camera.h"
#include "character.h" // Include the character header

extern Character player; // Reference to the player character

// Function to initialize the camera
void initCamera() {
    // Set initial camera position and settings
}

// Function to update the camera position
void updateCamera() {
    // Define camera parameters
    vec3 eye = {player.x, player.y + 5.0f, player.z + 5.0f};
    vec3 center = {player.x, player.y, player.z};
    vec3 up = {0.0f, 1.0f, 0.0f};

    // Create view matrix
    mat4 view;
    glm_lookat(eye, center, up, view);

    // We'll use this view matrix in the shader instead of calling glMultMatrixf
}

// Function to apply camera transformations for rendering
void applyCamera() {
    updateCamera(); // Update the camera position
} 