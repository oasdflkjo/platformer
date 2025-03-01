#include "world.h"
#include "character.h"
#include "input.h"
#include <GLFW/glfw3.h>
#include <math.h> // Include math.h for fabs
#include "cgltf.h"
#include "glb_loader.h" // Include the GLB loader header

// Define the player variable
Character player = {0.0f, 0.5f, 0.0f}; // Set y to 0.5 to place the cube on top of the plane

// Function to initialize the game world
void initWorld() {
    // Initialize game state, load levels, etc.

    // Load a GLB file
    loadGLB("assets/model.glb");
}

// Function to update the game world
void updateWorld() {
    int count;
    const float* axes = getControllerAxes(&count);

    if (count >= 2) { // Assuming the first two axes are for movement
        float moveSpeed = 1.4f / 60.0f; // Walking speed in meters per frame (assuming 60 FPS)
        float deadzone = 0.1f; // Define a deadzone threshold

        // Apply deadzone for x-axis
        if (fabs(axes[0]) > deadzone) {
            player.x += axes[0] * moveSpeed;
        }

        // Apply deadzone for z-axis
        if (fabs(axes[1]) > deadzone) {
            player.z += axes[1] * moveSpeed;
        }
    }
}

// Function to render the game world
void renderWorld() {
    // Draw the grid on the x-z plane
    glColor3f(0.5f, 0.5f, 0.5f); // Set grid color to gray
    glBegin(GL_LINES);
    for (int i = -10; i <= 10; i++) {
        // Lines parallel to the z-axis
        glVertex3f(i, 0, -10);
        glVertex3f(i, 0, 10);
        // Lines parallel to the x-axis
        glVertex3f(-10, 0, i);
        glVertex3f(10, 0, i);
    }
    glEnd();

    // Draw the player as a cube
    glColor3f(1.0f, 1.0f, 1.0f); // Set player color to white
    glBegin(GL_QUADS);
    // Front face
    glVertex3f(player.x - 0.5f, player.y - 0.5f, player.z + 0.5f);
    glVertex3f(player.x + 0.5f, player.y - 0.5f, player.z + 0.5f);
    glVertex3f(player.x + 0.5f, player.y + 0.5f, player.z + 0.5f);
    glVertex3f(player.x - 0.5f, player.y + 0.5f, player.z + 0.5f);
    // Back face
    glVertex3f(player.x - 0.5f, player.y - 0.5f, player.z - 0.5f);
    glVertex3f(player.x + 0.5f, player.y - 0.5f, player.z - 0.5f);
    glVertex3f(player.x + 0.5f, player.y + 0.5f, player.z - 0.5f);
    glVertex3f(player.x - 0.5f, player.y + 0.5f, player.z - 0.5f);
    // Other faces...
    glEnd();
} 