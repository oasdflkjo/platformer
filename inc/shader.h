#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <cglm/cglm.h>

typedef struct {
    unsigned int ID;
} Shader;

// Initialize shader from vertex and fragment shader files
void shader_init(Shader* shader, const char* vertexPath, const char* fragmentPath);

// Use the shader program
void shader_use(Shader* shader);

// Utility uniform functions
void shader_set_bool(Shader* shader, const char* name, bool value);
void shader_set_int(Shader* shader, const char* name, int value);
void shader_set_float(Shader* shader, const char* name, float value);
void shader_set_vec3(Shader* shader, const char* name, vec3 value);
void shader_set_mat4(Shader* shader, const char* name, mat4 value);

// Add this function declaration
void shader_get_mat4(Shader* shader, const char* name, mat4 dest);

#endif 