// input.c

#include <stdio.h>
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
int isJoystickPresent(int joystick) {
    return glfwJoystickPresent(joystick);
}

// Get joystick axes
const float* getJoystickAxes(int joystick, int* count) {
    return glfwGetJoystickAxes(joystick, count);
}

// Get joystick buttons
const unsigned char* getJoystickButtons(int joystick, int* count) {
    return glfwGetJoystickButtons(joystick, count);
}

// Debug print for joystick inputs
void debugPrintJoystickInputs(int joystick) {
    if (isJoystickPresent(joystick)) {
        int buttonCount;
        const unsigned char* buttons = getJoystickButtons(joystick, &buttonCount);

        // Print only pressed buttons
        for (int i = 0; i < buttonCount; ++i) {
            if (buttons[i] == GLFW_PRESS) {
                // printf("Button %d: Pressed\n", i); // Removed print statement
            }
        }
    } else {
        // printf("Joystick %d not present.\n", joystick); // Removed print statement
    }
}