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

// Add the following line to declare the debug function
void debugPrintJoystickInputs(int joystick);

// Handle input
void handle_input();

// Check if a specific button is pressed
bool isButtonPressed(int buttonID);

// Define button constants for PS5 controller
#define BUTTON_X 1       // X button (cross)
#define BUTTON_CIRCLE 2  // Circle button
#define BUTTON_SQUARE 0  // Square button
#define BUTTON_TRIANGLE 3 // Triangle button

#endif // INPUT_H