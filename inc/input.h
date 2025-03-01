// input.h

#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

// Initialize input system
void initInput(GLFWwindow* window);

// Update input states
void updateInput();

// Check if a specific key is pressed
int isKeyPressed(int key);

// Check if a joystick is present
int isJoystickPresent(int joystick);

// Get joystick axes
const float* getJoystickAxes(int joystick, int* count);

// Get joystick buttons
const unsigned char* getJoystickButtons(int joystick, int* count);

// Add the following line to declare the debug function
void debugPrintJoystickInputs(int joystick);

#endif // INPUT_H