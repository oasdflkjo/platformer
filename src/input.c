// input.c

#include <stdio.h>
#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "input.h"
#include "logging.h"

// Define this module for logging
LOG_MODULE_DEFINE(__FILE__, false);

// Store the window reference
static GLFWwindow* g_window = NULL;

// Add these global variables to track button states
static bool currentButtonStates[16] = {false};
static bool previousButtonStates[16] = {false};

// Initialize input system
void initInput(GLFWwindow* window) {
    g_window = window;
    LOG("Input system initialized");
}

// Update input states
void updateInput() {
    // Store previous button states
    for (int i = 0; i < 16; i++) {
        previousButtonStates[i] = currentButtonStates[i];
    }
    
    // Update current button states
    int count;
    const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
    if (buttons && count > 0) {
        for (int i = 0; i < count && i < 16; i++) {
            currentButtonStates[i] = buttons[i] == GLFW_PRESS;
        }
    }
    
    // Poll for and process events
    glfwPollEvents();
}

// Check if a specific key is pressed
int isKeyPressed(int key) {
    if (g_window == NULL) {
        LOG("Window reference is NULL in isKeyPressed");
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
                LOG("Button %d: Pressed", i);
            }
        }
    } else {
        LOG("Joystick not present.");
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
    
    if (!buttons) {
        return false;
    }
    
    if (buttonID < 0 || buttonID >= count) {
        LOG("Invalid button ID: %d (max: %d)", buttonID, count-1);
        return false;
    }
    
    return buttons[buttonID] == 1;
}

// Debug functions with logging
void debugControllerButtons() {
    LOG("Controller buttons debug function called");
}

void debugAllControllerButtons() {
    LOG("All controller buttons debug function called");
}

// Check if a button was just pressed this frame (pressed now but not in previous frame)
bool isButtonJustPressed(int button) {
    if (button < 0 || button >= 16) {
        LOG("Invalid button ID in isButtonJustPressed: %d", button);
        return false; // Invalid button ID
    }
    
    return currentButtonStates[button] && !previousButtonStates[button];
}

// Check if a button was just released this frame (not pressed now but was in previous frame)
bool isButtonJustReleased(int button) {
    if (button < 0 || button >= 16) {
        LOG("Invalid button ID in isButtonJustReleased: %d", button);
        return false; // Invalid button ID
    }
    
    return !currentButtonStates[button] && previousButtonStates[button];
}