#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include <stdio.h>
#include "glb_loader.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>

// Global variable to store the loaded model
static cgltf_data* model = NULL;

// Global variables for textures
static GLuint* texture_ids = NULL;
static size_t texture_count = 0;

// Function to load a GLB file
void loadGLB(const char* filename) {
    cgltf_options options = {0};
    cgltf_result result = cgltf_parse_file(&options, filename, &model);
    
    if (result != cgltf_result_success) {
        printf("Failed to load GLB file: %s\n", filename);
        return;
    }
    
    // Load buffers
    result = cgltf_load_buffers(&options, model, filename);
    if (result != cgltf_result_success) {
        printf("Failed to load GLB buffers\n");
        cgltf_free(model);
        model = NULL;
        return;
    }
    
    printf("Successfully loaded GLB file: %s\n", filename);
}

// Function to load textures from the model
void loadTextures() {
    if (!model || texture_ids) return; // Already loaded or no model
    
    // Count images
    texture_count = model->images_count;
    if (texture_count == 0) {
        // If no textures in the model, we'll create a default yellow texture
        texture_count = 1;
        texture_ids = (GLuint*)malloc(texture_count * sizeof(GLuint));
        glGenTextures(texture_count, texture_ids);
        
        // Create a simple yellow texture
        glBindTexture(GL_TEXTURE_2D, texture_ids[0]);
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Create a simple 2x2 yellow texture
        unsigned char yellowTexture[4 * 3] = {
            255, 255, 0,  255, 255, 0,  // Yellow pixels
            255, 255, 0,  255, 255, 0
        };
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, yellowTexture);
        
        // Unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }
    
    // If we have images in the model, process them
    texture_ids = (GLuint*)malloc(texture_count * sizeof(GLuint));
    glGenTextures(texture_count, texture_ids);
    
    // Load each texture
    for (size_t i = 0; i < texture_count; i++) {
        cgltf_image* image = &model->images[i];
        
        // Bind texture
        glBindTexture(GL_TEXTURE_2D, texture_ids[i]);
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // For now, just create a yellow texture since we don't know how to parse the image data
        unsigned char yellowTexture[4 * 3] = {
            255, 255, 0,  255, 255, 0,  // Yellow pixels
            255, 255, 0,  255, 255, 0
        };
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, yellowTexture);
        
        // Unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

// Function to render the loaded GLB model
void renderGLBModel() {
    if (model == NULL) {
        // Model not loaded, render a simple cube as fallback
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow color
        
        // Draw a cube
        glBegin(GL_QUADS);
        // Front face
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        
        // Back face
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        
        // Top face
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        
        // Bottom face
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        
        // Right face
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        
        // Left face
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glEnd();
        
        return;
    }
    
    // Load textures if not already loaded
    if (!texture_ids && model->images_count > 0) {
        loadTextures();
    }
    
    // Enable texturing
    glEnable(GL_TEXTURE_2D);
    
    // Iterate through all meshes in the model
    for (size_t i = 0; i < model->meshes_count; i++) {
        cgltf_mesh* mesh = &model->meshes[i];
        
        // Iterate through all primitives in the mesh
        for (size_t j = 0; j < mesh->primitives_count; j++) {
            cgltf_primitive* primitive = &mesh->primitives[j];
            
            // Apply material if available
            if (primitive->material) {
                // Set material properties for yellow hazmat suit
                float yellow[] = {1.0f, 1.0f, 0.0f, 1.0f};
                float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
                float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
                float shininess = 30.0f;
                
                // Check if this is the visor part (usually has a different material)
                if (primitive->material->name && strstr(primitive->material->name, "visor")) {
                    // Set visor to black
                    glColor4f(0.1f, 0.1f, 0.1f, 0.8f);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
                    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0f);
                } else if (primitive->material->name && strstr(primitive->material->name, "glove")) {
                    // Set gloves to gray
                    glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
                    float gray[] = {0.5f, 0.5f, 0.5f, 1.0f};
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, gray);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, gray);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
                    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
                } else {
                    // Default yellow for the suit
                    glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, yellow);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, yellow);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
                    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
                }
                
                // Bind the yellow texture if available
                if (texture_ids && texture_count > 0) {
                    glBindTexture(GL_TEXTURE_2D, texture_ids[0]);
                }
            } else {
                // Default material properties
                float yellow[] = {1.0f, 1.0f, 0.0f, 1.0f};
                float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
                glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, yellow);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, yellow);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 30.0f);
            }
            
            // Begin drawing triangles
            glBegin(GL_TRIANGLES);
            
            // Get indices if available
            cgltf_accessor* indices = primitive->indices;
            size_t index_count = indices ? indices->count : 0;
            
            // Get position accessor
            cgltf_accessor* positions = NULL;
            cgltf_accessor* texcoords = NULL;
            cgltf_accessor* normals = NULL;
            
            // Find all required attributes
            for (size_t k = 0; k < primitive->attributes_count; k++) {
                if (primitive->attributes[k].type == cgltf_attribute_type_position) {
                    positions = primitive->attributes[k].data;
                } else if (primitive->attributes[k].type == cgltf_attribute_type_texcoord) {
                    texcoords = primitive->attributes[k].data;
                } else if (primitive->attributes[k].type == cgltf_attribute_type_normal) {
                    normals = primitive->attributes[k].data;
                }
            }
            
            if (positions) {
                // If we have indices, use them
                if (indices) {
                    for (size_t k = 0; k < index_count; k++) {
                        uint32_t idx = cgltf_accessor_read_index(indices, k);
                        
                        // Set texture coordinate if available
                        if (texcoords) {
                            float tc[2];
                            cgltf_accessor_read_float(texcoords, idx, tc, 2);
                            glTexCoord2f(tc[0], tc[1]);
                        }
                        
                        // Set normal if available
                        if (normals) {
                            float n[3];
                            cgltf_accessor_read_float(normals, idx, n, 3);
                            glNormal3f(n[0], n[1], n[2]);
                        }
                        
                        // Set vertex position
                        float pos[3];
                        cgltf_accessor_read_float(positions, idx, pos, 3);
                        glVertex3f(pos[0], pos[1], pos[2]);
                    }
                } else {
                    // Otherwise, use vertices directly
                    for (size_t k = 0; k < positions->count; k++) {
                        // Set texture coordinate if available
                        if (texcoords) {
                            float tc[2];
                            cgltf_accessor_read_float(texcoords, k, tc, 2);
                            glTexCoord2f(tc[0], tc[1]);
                        }
                        
                        // Set normal if available
                        if (normals) {
                            float n[3];
                            cgltf_accessor_read_float(normals, k, n, 3);
                            glNormal3f(n[0], n[1], n[2]);
                        }
                        
                        // Set vertex position
                        float pos[3];
                        cgltf_accessor_read_float(positions, k, pos, 3);
                        glVertex3f(pos[0], pos[1], pos[2]);
                    }
                }
            }
            
            glEnd();
            
            // Unbind texture
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    
    // Disable texturing
    glDisable(GL_TEXTURE_2D);
}

// Function to clean up resources
void cleanupGLB() {
    if (model) {
        cgltf_free(model);
        model = NULL;
    }
    
    // Free texture resources
    if (texture_ids) {
        glDeleteTextures(texture_count, texture_ids);
        free(texture_ids);
        texture_ids = NULL;
        texture_count = 0;
    }
} 