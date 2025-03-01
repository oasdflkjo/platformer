#include "png_decoder.h"
#include <stdlib.h>

// Load an image from a file path
Image* load_image(const char* filepath) {
    Image* image = (Image*)malloc(sizeof(Image));
    if (!image) return NULL;
    
    // Use stb_image to load the file
    image->data = stbi_load(filepath, &image->width, &image->height, &image->channels, 0);
    
    // Check if loading was successful
    if (!image->data) {
        free(image);
        return NULL;
    }
    
    return image;
}

// Free the memory allocated for an image
void free_image(Image* image) {
    if (image) {
        if (image->data) {
            stbi_image_free(image->data);
        }
        free(image);
    }
} 