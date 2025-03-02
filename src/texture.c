#include "../inc/texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "../external/stb/stb_image.h"

// BMP file header structure
#pragma pack(push, 1)
typedef struct {
    unsigned char signature[2];
    unsigned int fileSize;
    unsigned int reserved;
    unsigned int dataOffset;
} BMPHeader;

typedef struct {
    unsigned int size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bitsPerPixel;
    unsigned int compression;
    unsigned int imageSize;
    int xPixelsPerMeter;
    int yPixelsPerMeter;
    unsigned int colorsUsed;
    unsigned int colorsImportant;
} BMPInfoHeader;
#pragma pack(pop)

unsigned int texture_load_bmp(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("Failed to open BMP file: %s\n", path);
        return 0;
    }
    
    // Read the BMP header
    BMPHeader header;
    fread(&header, sizeof(BMPHeader), 1, file);
    
    // Check if it's a valid BMP file
    if (header.signature[0] != 'B' || header.signature[1] != 'M') {
        printf("Not a valid BMP file: %s\n", path);
        fclose(file);
        return 0;
    }
    
    // Read the BMP info header
    BMPInfoHeader infoHeader;
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, file);
    
    // Check if it's a supported format (24 or 32 bits per pixel)
    if (infoHeader.bitsPerPixel != 24 && infoHeader.bitsPerPixel != 32) {
        printf("Unsupported BMP format (only 24 or 32 bpp supported): %s\n", path);
        fclose(file);
        return 0;
    }
    
    // Calculate row size (including padding)
    int rowSize = ((infoHeader.width * infoHeader.bitsPerPixel / 8) + 3) & ~3;
    
    // Allocate memory for the image data
    unsigned char* data = (unsigned char*)malloc(rowSize * abs(infoHeader.height));
    if (!data) {
        printf("Failed to allocate memory for BMP data: %s\n", path);
        fclose(file);
        return 0;
    }
    
    // Seek to the beginning of the pixel data
    fseek(file, header.dataOffset, SEEK_SET);
    
    // Read the pixel data
    fread(data, rowSize, abs(infoHeader.height), file);
    fclose(file);
    
    // Create OpenGL texture
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Convert BGR(A) to RGB(A) and flip vertically
    int width = infoHeader.width;
    int height = abs(infoHeader.height);
    int bytesPerPixel = infoHeader.bitsPerPixel / 8;
    unsigned char* flippedData = (unsigned char*)malloc(width * height * bytesPerPixel);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int srcIndex = y * rowSize + x * bytesPerPixel;
            int destIndex = (height - 1 - y) * width * bytesPerPixel + x * bytesPerPixel;
            
            // Swap BGR to RGB
            flippedData[destIndex + 0] = data[srcIndex + 2]; // R
            flippedData[destIndex + 1] = data[srcIndex + 1]; // G
            flippedData[destIndex + 2] = data[srcIndex + 0]; // B
            
            // Copy alpha channel if present
            if (bytesPerPixel == 4) {
                flippedData[destIndex + 3] = data[srcIndex + 3]; // A
            }
        }
    }
    
    // Upload texture data
    GLenum format = (bytesPerPixel == 3) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, flippedData);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Free memory
    free(data);
    free(flippedData);
    
    return textureID;
}

// Add this function to print texture info
void texture_print_info(unsigned int textureID) {
    int width, height;
    glBindTexture(GL_TEXTURE_2D, textureID);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    
    printf("Texture ID: %u, Width: %d, Height: %d\n", textureID, width, height);
}

// Function to load a PNG texture
unsigned int texture_load_png(const char* path) {
    printf("Loading PNG texture: %s\n", path);
    
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    
    if (data) {
        // Create OpenGL texture
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Free image data
        stbi_image_free(data);
        
        printf("Loaded PNG texture: %s (ID: %u, Size: %dx%d)\n", path, textureID, width, height);
        
        return textureID;
    } else {
        printf("Failed to load PNG texture: %s\n", path);
        return 0;
    }
}

// Create a simple test texture with a clear pattern
unsigned int create_test_texture() {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Create a 64x64 checkerboard texture with more vibrant colors
    unsigned char checkerboard[64 * 64 * 4];
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            // Create a more interesting pattern
            int blockX = x / 8;
            int blockY = y / 8;
            bool isCheckerboard = (blockX + blockY) % 2 == 0;
            
            int index = (y * 64 + x) * 4;
            
            if (isCheckerboard) {
                // Vibrant color based on position
                checkerboard[index + 0] = (x * 4) % 255;     // R
                checkerboard[index + 1] = (y * 4) % 255;     // G
                checkerboard[index + 2] = ((x+y) * 2) % 255; // B
                checkerboard[index + 3] = 255;               // A
            } else {
                // Darker color for contrast
                checkerboard[index + 0] = 50;  // R
                checkerboard[index + 1] = 50;  // G
                checkerboard[index + 2] = 50;  // B
                checkerboard[index + 3] = 255; // A
            }
            
            // Add a border around each cell
            if (x % 8 == 0 || y % 8 == 0) {
                checkerboard[index + 0] = 255; // R
                checkerboard[index + 1] = 255; // G
                checkerboard[index + 2] = 255; // B
                checkerboard[index + 3] = 255; // A
            }
        }
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkerboard);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    printf("Created test checkerboard texture (ID: %u)\n", textureID);
    
    return textureID;
}

// Function to load a texture (generic wrapper that selects the appropriate loader)
unsigned int texture_load(const char* path) {
    // Check file extension to determine which loader to use
    const char* extension = strrchr(path, '.');
    if (!extension) {
        printf("Error: Could not determine file extension for %s\n", path);
        return 0;
    }
    
    // Convert extension to lowercase for comparison
    char ext_lower[10] = {0};
    int i = 0;
    while (extension[i] && i < 9) {
        ext_lower[i] = tolower(extension[i]);
        i++;
    }
    
    // Select appropriate loader based on file extension
    if (strcmp(ext_lower, ".bmp") == 0) {
        return texture_load_bmp(path);
    } else if (strcmp(ext_lower, ".png") == 0) {
        return texture_load_png(path);
    } else {
        // For other formats, try to use stb_image
        int width, height, channels;
        unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
        if (!data) {
            printf("Failed to load texture: %s\n", path);
            return 0;
        }
        
        // Create OpenGL texture
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Determine format based on channels
        GLenum format;
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;
        else {
            printf("Unsupported number of channels: %d\n", channels);
            stbi_image_free(data);
            return 0;
        }
        
        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Free image data
        stbi_image_free(data);
        
        return textureID;
    }
}
