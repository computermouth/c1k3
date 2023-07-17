#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

void mapMeshesToBooleanArray(const char* filePath, bool map[][3][3]) {
    // Load the glTF file using cgltf
    cgltf_data* data;
    cgltf_options options = {0};
    cgltf_result result = cgltf_parse_file(&options, filePath, &data);

    if (result != cgltf_result_success) {
        printf("Failed to parse glTF file: %s\n", filePath);
        return;
    }

    // Load the glTF file and access the meshes
    cgltf_load_buffers(&options, data, filePath);
    printf("nodes: %d\n", (int)data->nodes_count);

    for (size_t i = 0; i < data->nodes_count; ++i) {
        
        cgltf_node * n = &(data->nodes[i]);
        
        cgltf_float m[16] = {0};
        cgltf_node_transform_world(n, m);
        
        cgltf_mesh* mesh = n->mesh;

            // Calculate the dimensions of the primitive
            int minPos[3] = {INT_MAX, INT_MAX, INT_MAX};
            int maxPos[3] = {INT_MIN, INT_MIN, INT_MIN};

        // Iterate over the primitives of the mesh
        for (size_t j = 0; j < mesh->primitives_count; ++j) {
            cgltf_primitive* primitive = &mesh->primitives[j];

            // Access the positions of the primitive
            cgltf_accessor* positionAccessor = primitive->attributes[0].data;
            cgltf_buffer_view* positionView = positionAccessor->buffer_view;
            cgltf_buffer* positionBuffer = positionView->buffer;
            float* positions = (float*)(positionBuffer->data + positionView->offset + positionAccessor->offset);

            size_t positionCount = positionAccessor->count;
            size_t positionStride = positionAccessor->stride / sizeof(float);

            // Apply the transformation matrix to each vertex position
            for (size_t k = 0; k < positionCount; ++k) {
                float x = positions[k * positionStride];
                float y = positions[k * positionStride + 1];
                float z = positions[k * positionStride + 2];

                // Apply the transformation matrix to the vertex position
                float transformedX = x * m[0] + y * m[4] + z * m[8] + m[12];
                float transformedY = x * m[1] + y * m[5] + z * m[9] + m[13];
                float transformedZ = x * m[2] + y * m[6] + z * m[10] + m[14];

                // Calculate the integer indices of the transformed vertex position
                int posX = (int)transformedX;
                int posY = (int)transformedY;
                int posZ = (int)transformedZ;

                // Update the minimum and maximum positions
                minPos[0] = (int)fmin(minPos[0], posX);
                minPos[1] = (int)fmin(minPos[1], posY);
                minPos[2] = (int)fmin(minPos[2], posZ);

                maxPos[0] = (int)fmax(maxPos[0], posX);
                maxPos[1] = (int)fmax(maxPos[1], posY);
                maxPos[2] = (int)fmax(maxPos[2], posZ);
            }
        }

            // Set the corresponding indices in the boolean map array
            for (int x = minPos[0]; x < maxPos[0]; ++x) {
                for (int y = minPos[1]; y < maxPos[1]; ++y) {
                    for (int z = minPos[2]; z < maxPos[2]; ++z) {
                        map[x][y][z] = true;
                    }
                }
            }
    }

    // Clean up cgltf resources
    cgltf_free(data);
}

int main() {
    bool map[3][3][3] = { false };

    // Call the function to map the meshes to the boolean array
    mapMeshesToBooleanArray("untitled9.gltf", map);
            
            
            // Set the corresponding indices in the boolean map array
            for (int z = 0; z < 3; z++) {
                printf("{\n");
                printf("  {%d,%d,%d}\n", map[0][0][z], map[0][1][z], map[0][2][z]);
                printf("  {%d,%d,%d}\n", map[1][0][z], map[1][1][z], map[1][2][z]);
                printf("  {%d,%d,%d}\n", map[2][0][z], map[0][1][z], map[2][2][z]);
                printf("}\n");
            }

    return 0;
}
