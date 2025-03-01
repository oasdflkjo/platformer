#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void model_load(Model* model, const char* filename) {
    // Parse the GLB file
    cgltf_options options = {0};
    cgltf_result result = cgltf_parse_file(&options, filename, &model->data);
    
    if (result != cgltf_result_success) {
        printf("Failed to parse GLB file: %s\n", filename);
        return;
    }
    
    // Load buffers
    result = cgltf_load_buffers(&options, model->data, filename);
    if (result != cgltf_result_success) {
        printf("Failed to load GLB buffers\n");
        cgltf_free(model->data);
        model->data = NULL;
        return;
    }
    
    // Count meshes
    model->meshCount = model->data->meshes_count;
    model->meshes = (Mesh*)malloc(model->meshCount * sizeof(Mesh));
    
    // Process each mesh
    for (int i = 0; i < model->meshCount; i++) {
        cgltf_mesh* mesh = &model->data->meshes[i];
        
        // For simplicity, we'll only process the first primitive of each mesh
        if (mesh->primitives_count > 0) {
            cgltf_primitive* primitive = &mesh->primitives[0];
            
            // Find position, normal, and texcoord attributes
            cgltf_accessor* positions = NULL;
            cgltf_accessor* normals = NULL;
            cgltf_accessor* texcoords = NULL;
            
            for (size_t j = 0; j < primitive->attributes_count; j++) {
                if (primitive->attributes[j].type == cgltf_attribute_type_position) {
                    positions = primitive->attributes[j].data;
                } else if (primitive->attributes[j].type == cgltf_attribute_type_normal) {
                    normals = primitive->attributes[j].data;
                } else if (primitive->attributes[j].type == cgltf_attribute_type_texcoord) {
                    texcoords = primitive->attributes[j].data;
                }
            }
            
            if (positions) {
                // Get indices
                cgltf_accessor* indices = primitive->indices;
                
                // Allocate vertex data
                int vertexCount = positions->count;
                int stride = 8; // 3 for position, 3 for normal, 2 for texcoord
                model->meshes[i].vertexCount = vertexCount;
                model->meshes[i].vertices = (float*)malloc(vertexCount * stride * sizeof(float));
                
                // Fill vertex data
                for (int j = 0; j < vertexCount; j++) {
                    // Position
                    cgltf_accessor_read_float(positions, j, &model->meshes[i].vertices[j * stride], 3);
                    
                    // Normal (if available)
                    if (normals) {
                        cgltf_accessor_read_float(normals, j, &model->meshes[i].vertices[j * stride + 3], 3);
                    } else {
                        model->meshes[i].vertices[j * stride + 3] = 0.0f;
                        model->meshes[i].vertices[j * stride + 4] = 1.0f;
                        model->meshes[i].vertices[j * stride + 5] = 0.0f;
                    }
                    
                    // Texcoord (if available)
                    if (texcoords) {
                        cgltf_accessor_read_float(texcoords, j, &model->meshes[i].vertices[j * stride + 6], 2);
                    } else {
                        model->meshes[i].vertices[j * stride + 6] = 0.0f;
                        model->meshes[i].vertices[j * stride + 7] = 0.0f;
                    }
                }
                
                // Process indices
                if (indices) {
                    model->meshes[i].indexCount = indices->count;
                    model->meshes[i].indices = (unsigned int*)malloc(indices->count * sizeof(unsigned int));
                    
                    for (int j = 0; j < indices->count; j++) {
                        model->meshes[i].indices[j] = cgltf_accessor_read_index(indices, j);
                    }
                } else {
                    // If no indices, create sequential indices
                    model->meshes[i].indexCount = vertexCount;
                    model->meshes[i].indices = (unsigned int*)malloc(vertexCount * sizeof(unsigned int));
                    
                    for (int j = 0; j < vertexCount; j++) {
                        model->meshes[i].indices[j] = j;
                    }
                }
                
                // Set default color (yellow for hazmat suit)
                model->meshes[i].color[0] = 1.0f;
                model->meshes[i].color[1] = 1.0f;
                model->meshes[i].color[2] = 0.0f;
                
                // Create VAO, VBO, EBO
                glGenVertexArrays(1, &model->meshes[i].VAO);
                glGenBuffers(1, &model->meshes[i].VBO);
                glGenBuffers(1, &model->meshes[i].EBO);
                
                // Bind VAO
                glBindVertexArray(model->meshes[i].VAO);
                
                // Bind VBO and copy vertex data
                glBindBuffer(GL_ARRAY_BUFFER, model->meshes[i].VBO);
                glBufferData(GL_ARRAY_BUFFER, vertexCount * stride * sizeof(float), model->meshes[i].vertices, GL_STATIC_DRAW);
                
                // Bind EBO and copy index data
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->meshes[i].EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->meshes[i].indexCount * sizeof(unsigned int), model->meshes[i].indices, GL_STATIC_DRAW);
                
                // Set vertex attribute pointers
                // Position attribute
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                
                // Normal attribute
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(1);
                
                // Texcoord attribute
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(6 * sizeof(float)));
                glEnableVertexAttribArray(2);
                
                // Unbind VAO
                glBindVertexArray(0);
                
                // Set texture flag (for now, no textures)
                model->meshes[i].hasTexture = false;
            }
        }
    }
    
    printf("Successfully loaded model: %s with %d meshes\n", filename, model->meshCount);
}

void model_render(Model* model, Shader* shader) {
    for (int i = 0; i < model->meshCount; i++) {
        // Set material properties
        shader_set_vec3(shader, "objectColor", model->meshes[i].color);
        shader_set_bool(shader, "useTexture", model->meshes[i].hasTexture);
        
        if (model->meshes[i].hasTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, model->meshes[i].texture);
        }
        
        // Draw mesh
        glBindVertexArray(model->meshes[i].VAO);
        glDrawElements(GL_TRIANGLES, model->meshes[i].indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void model_cleanup(Model* model) {
    for (int i = 0; i < model->meshCount; i++) {
        glDeleteVertexArrays(1, &model->meshes[i].VAO);
        glDeleteBuffers(1, &model->meshes[i].VBO);
        glDeleteBuffers(1, &model->meshes[i].EBO);
        
        if (model->meshes[i].hasTexture) {
            glDeleteTextures(1, &model->meshes[i].texture);
        }
        
        free(model->meshes[i].vertices);
        free(model->meshes[i].indices);
    }
    
    free(model->meshes);
    cgltf_free(model->data);
} 