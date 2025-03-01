// Test file to identify which header is including OpenGL headers
#include <stdio.h>

// Include each header one by one and see which one causes the error
#include "world.h"
// #include "character.h"
// #include "input.h"
// Uncomment these one by one to test

// Rename this function to avoid conflict with the main() in main.c
int test_main() {
    printf("Headers included successfully\n");
    return 0;
} 