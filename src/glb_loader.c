#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include <stdio.h>

void loadGLB(const char* filename) {
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, filename, &data);

    if (result == cgltf_result_success) {
        result = cgltf_load_buffers(&options, data, filename);
        if (result == cgltf_result_success) {
            // Process the loaded data
            printf("GLB file loaded successfully.\n");
        }
        cgltf_free(data);
    } else {
        printf("Failed to load GLB file: %s\n", filename);
    }
} 