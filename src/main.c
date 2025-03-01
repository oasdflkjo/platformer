// main.c

#define _USE_MATH_DEFINES // Allow the use of M_PI
#include <math.h>
#include <GLFW/glfw3.h>
#include "input.h"
#include "world.h"
#include "camera.h"
#include "cgltf.h"

// Window dimensions
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Platformer Engine", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set up perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspectRatio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    float fovY = 45.0f; // Field of view
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    float top = nearPlane * tanf(fovY * 0.5f * (M_PI / 180.0f));
    float right = top * aspectRatio;

    glFrustum(-right, right, -top, top, nearPlane, farPlane);
    glMatrixMode(GL_MODELVIEW);

    // Initialize input system
    initInput(window);
    
    // Initialize world and camera
    initWorld();
    initCamera();

    // Timing variables
    const double fixedTimeStep = 1.0 / 60.0; // 60 updates per second
    double previousTime = glfwGetTime();
    double accumulator = 0.0;

    // Main game loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate elapsed time
        double currentTime = glfwGetTime();
        double frameTime = currentTime - previousTime;
        previousTime = currentTime;
        accumulator += frameTime;

        // Update game logic at fixed intervals
        while (accumulator >= fixedTimeStep) {
            updateInput();
            updateWorld();
            applyCamera();
            accumulator -= fixedTimeStep;
        }

        // Render the scene
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderWorld();
        glfwSwapBuffers(window);
    }

    // Clean up and exit
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}