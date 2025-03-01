#ifndef CHARACTER_H
#define CHARACTER_H

#include <stdbool.h> // Add this for bool type

typedef struct {
    float x, y, z;     // Position of the character in 3D
    float velocityY;   // Vertical velocity for jumping
    bool isGrounded;   // Whether the character is on the ground
} Character;

// Declare the player variable
extern Character player;

#endif // CHARACTER_H 