// This file is likely redundant now that you're using stb_image
// You should remove it if it's just a wrapper around stb functionality
// or refactor it to use stb_image internally

#ifndef PNG_DECODER_H
#define PNG_DECODER_H

#include "../external/stb/stb_image.h"

typedef struct {
    unsigned char* data;
    int width;
    int height;
    int channels;
} Image;

// Load an image from a file path
// Returns an Image struct with the image data
// Returns NULL if loading fails
Image* load_image(const char* filepath);

// Free the memory allocated for an image
void free_image(Image* image);

#endif // PNG_DECODER_H 