// input.h

#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>
#include <stdbool.h>

// Function declarations
void initInput(GLFWwindow* window);
void updateInput();
int isKeyPressed(int key);
int isControllerConnected();
const float* getControllerAxes(int* count);

// Check if a joystick is present
int isJoystickPresent(int joystick);

// Get joystick axes
const float* getJoystickAxes(int joystick, int* count);

// Get joystick buttons
const unsigned char* getJoystickButtons(int joystick, int* count);

// Debug function to print joystick inputs
void debugPrintJoystickInputs(int joystick);

// Handle input
void handle_input();

// Check if a specific button is pressed
bool isButtonPressed(int buttonID);

// Get window reference
GLFWwindow* getWindow(void);

// Controller button definitions - unified
// These are the standard Xbox-style button mappings
// Different controllers may have different mappings
#define BUTTON_A        0
#define BUTTON_B        1
#define BUTTON_X        2
#define BUTTON_Y        3
#define BUTTON_LB       4
#define BUTTON_RB       5
#define BUTTON_BACK     6
#define BUTTON_START    7
#define BUTTON_L3       8
#define BUTTON_R3       9
#define BUTTON_DPAD_UP  10
#define BUTTON_DPAD_RIGHT 11
#define BUTTON_DPAD_DOWN 12
#define BUTTON_DPAD_LEFT 13

// PS5 controller button aliases (based on actual mappings)
#define BUTTON_CROSS    1  // X button on PS5 controller
#define BUTTON_CIRCLE   2  // Circle button on PS5 controller
#define BUTTON_SQUARE   0  // Square button on PS5 controller
#define BUTTON_TRIANGLE 3  // Triangle button on PS5 controller
#define BUTTON_L1       4  // Same as BUTTON_LB for Xbox controllers

// Check if a button was just pressed this frame
bool isButtonJustPressed(int button);

// Check if a button was just released this frame
bool isButtonJustReleased(int button);

// Clean up input resources
void cleanupInput(void);

// Add this function declaration to input.h
void debugControllerButtons(void);

// Add this function declaration
void debugAllControllerButtons(void);

#endif // INPUT_H