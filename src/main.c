// main.c

#define _USE_MATH_DEFINES // Allow the use of M_PI
#include <math.h>
#include <glad/glad.h> // GLAD must come before GLFW
#include <GLFW/glfw3.h>
#include <stdio.h>
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
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Platformer Engine", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    // Make the window's context current
    glfwMakeContextCurrent(window);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }
    
    // Initialize input system
    initInput(window);
    
    // Initialize world
    initWorld();
    
    // Timing variables
    const double fixedTimeStep = 1.0 / 60.0;
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
            accumulator -= fixedTimeStep;
        }
        
        // Get window size for aspect ratio
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspectRatio = (float)width / (float)height;
        
        // Render the scene
        renderWorld(aspectRatio);
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        glfwSwapInterval(1); // Enable VSync to limit frame rate to monitor refresh rate
    }
    
    // Clean up resources
    cleanupWorld();
    
    // Clean up and exit
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}