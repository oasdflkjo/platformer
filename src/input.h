#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <GLFW/glfw3.h>

// Button definitions
#define BUTTON_CROSS     0
#define BUTTON_CIRCLE    1
#define BUTTON_SQUARE    2
#define BUTTON_TRIANGLE  3
#define BUTTON_L1        4
#define BUTTON_R1        5
#define BUTTON_L2        6
#define BUTTON_R2        7
#define BUTTON_SHARE     8
#define BUTTON_OPTIONS   9
#define BUTTON_L3        10
#define BUTTON_R3        11
#define BUTTON_PS        12
#define BUTTON_TOUCHPAD  13
#define BUTTON_DPAD_UP   14
#define BUTTON_DPAD_RIGHT 15
#define BUTTON_DPAD_DOWN 16
#define BUTTON_DPAD_LEFT 17

// Function declarations
bool isButtonPressed(int button);
const float* getControllerAxes(int* count);
void initInput(GLFWwindow* window);
void updateInput(void);
void cleanupInput(void);

#endif // INPUT_H 