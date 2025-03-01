#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include "cgltf.h"
#include "shader.h"

typedef struct {
    unsigned int VAO, VBO, EBO;
    unsigned int texture;
    int vertexCount;
    int indexCount;
    float* vertices;
    unsigned int* indices;
    vec3 color;
    bool hasTexture;
} Mesh;

typedef struct {
    Mesh* meshes;
    int meshCount;
    cgltf_data* data;
} Model;

// Load a model from a GLB file
void model_load(Model* model, const char* filename);

// Render the model using the specified shader
void model_render(Model* model, Shader* shader);

// Clean up model resources
void model_cleanup(Model* model);

#endif 