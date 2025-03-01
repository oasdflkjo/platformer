// input.c

#include <stdio.h>
#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "input.h"

// Store the window reference
static GLFWwindow* g_window = NULL;

// Initialize input system
void initInput(GLFWwindow* window) {
    g_window = window;
    // Additional initialization if needed
}

// Update input states
void updateInput() {
    // Poll for and process events
    glfwPollEvents();
}

// Check if a specific key is pressed
int isKeyPressed(int key) {
    if (g_window == NULL) {
        return 0;
    }
    return glfwGetKey(g_window, key) == GLFW_PRESS;
}

// Check if a joystick is present
int isControllerConnected() {
    return glfwJoystickPresent(GLFW_JOYSTICK_1);
}

// Get joystick axes
const float* getControllerAxes(int* count) {
    return glfwGetJoystickAxes(GLFW_JOYSTICK_1, count);
}

// Get joystick buttons
const unsigned char* getJoystickButtons(int joystick, int* count) {
    return glfwGetJoystickButtons(joystick, count);
}

// Debug print for joystick inputs
void debugPrintJoystickInputs(int joystick) {
    if (isControllerConnected()) {
        int buttonCount;
        const unsigned char* buttons = getJoystickButtons(joystick, &buttonCount);

        // Print only pressed buttons
        for (int i = 0; i < buttonCount; ++i) {
            if (buttons[i] == GLFW_PRESS) {
                // printf("Button %d: Pressed\n", i); // Removed print statement
            }
        }
    } else {
        // printf("Joystick not present.\n"); // Removed print statement
    }
}

void handle_input() {
    // existing input handling...

    // Comment out or remove the animation input handling if not needed
    // if (isKeyPressed(KEY_ANIMATION)) {
    //     trigger_character_animation();
    // }

    // existing input handling...
}

// Add this function to check if a specific button is pressed
bool isButtonPressed(int buttonID) {
    int count;
    const unsigned char* buttons = getJoystickButtons(GLFW_JOYSTICK_1, &count);
    
    if (buttonID < count) {
        return buttons[buttonID] == 1;
    }
    
    return false;
}

// Add this function to help debug controller buttons
void debugControllerButtons() {
    int count;
    const unsigned char* buttons = getJoystickButtons(GLFW_JOYSTICK_1, &count);
    
    // Print all pressed buttons
    for (int i = 0; i < count; i++) {
        if (buttons[i] == 1) {
            printf("Button %d is pressed\n", i);
        }
    }
}