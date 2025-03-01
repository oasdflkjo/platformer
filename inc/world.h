#ifndef WORLD_H
#define WORLD_H

#include <GLFW/glfw3.h>

// Function to initialize the game world
void initWorld(GLFWwindow* win);

// Function to update the game world
void updateWorld();

// Function to render the game world
void renderWorld(float aspectRatio);

// Function to draw the grid
void drawGrid();

// Function to clean up resources
void cleanupWorld();

#endif // WORLD_H 