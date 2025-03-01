#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Read file content into a string
char* read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");  // Open in binary mode
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        fclose(file);
        printf("Failed to allocate memory for file: %s\n", filename);
        return NULL;
    }
    
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    
    fclose(file);
    return buffer;
}

void shader_init(Shader* shader, const char* vertexPath, const char* fragmentPath) {
    // Read vertex shader code
    char* vertexCode = read_file(vertexPath);
    if (!vertexCode) {
        printf("Failed to read vertex shader file\n");
        return;
    }
    
    // Read fragment shader code
    char* fragmentCode = read_file(fragmentPath);
    if (!fragmentCode) {
        free(vertexCode);
        printf("Failed to read fragment shader file\n");
        return;
    }
    
    // Compile shaders
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];
    
    // Vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, (const char**)&vertexCode, NULL);
    glCompileShader(vertex);
    
    // Check for compilation errors
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        
        // Print the shader source for debugging
        printf("Vertex Shader Source:\n");
        int i = 0;
        const char* src = vertexCode;
        while (*src) {
            printf("%3d: %c [%d]\n", i++, *src, (int)*src);
            src++;
        }
    }
    
    // Fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (const char**)&fragmentCode, NULL);
    glCompileShader(fragment);
    
    // Check for compilation errors
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }
    
    // Shader program
    shader->ID = glCreateProgram();
    glAttachShader(shader->ID, vertex);
    glAttachShader(shader->ID, fragment);
    glLinkProgram(shader->ID);
    
    // Check for linking errors
    glGetProgramiv(shader->ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader->ID, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }
    
    // Delete shaders as they're linked into the program now
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    // Free memory
    free(vertexCode);
    free(fragmentCode);
}

void shader_use(Shader* shader) {
    glUseProgram(shader->ID);
}

void shader_set_bool(Shader* shader, const char* name, bool value) {
    glUniform1i(glGetUniformLocation(shader->ID, name), (int)value);
}

void shader_set_int(Shader* shader, const char* name, int value) {
    glUniform1i(glGetUniformLocation(shader->ID, name), value);
}

void shader_set_float(Shader* shader, const char* name, float value) {
    glUniform1f(glGetUniformLocation(shader->ID, name), value);
}

void shader_set_vec3(Shader* shader, const char* name, vec3 value) {
    glUniform3fv(glGetUniformLocation(shader->ID, name), 1, value);
}

void shader_set_mat4(Shader* shader, const char* name, mat4 value) {
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, name), 1, GL_FALSE, (float*)value);
} 