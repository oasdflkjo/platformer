#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

// Function to load a BMP texture
unsigned int texture_load_bmp(const char* path);

// Function to load a PNG texture
unsigned int texture_load_png(const char* path);

// Function to print texture info
void texture_print_info(unsigned int textureID);

// Function to create a test texture
unsigned int create_test_texture();

// Note: load_texture is now defined in texture_loader.h/c

#endif // TEXTURE_H 