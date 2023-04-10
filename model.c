
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "model.h"
#include "data.h"
#include "render.h"

model_collection_t model_load_container() {

    const uint8_t * data = data_models;
    model_collection_t m = {0};

    for (uint32_t i = 0; i < data_models_len;) {
        // let model_size = num_frames * num_verts * 3 + num_indices * 3
        uint32_t model_size = (data[i] * data[i+1] + data[i+2]) * 3;
        i += 3;
        m.len++;
        m.models = realloc(m.models, sizeof(model_t) * m.len);
        m.models[m.len - 1] = (model_t) {
            .data = (uint8_t *)(data + (i - 3)),
            .data_len = model_size,
        };
        i += model_size;
    }

    return m;
}

// todo, return different type, like entity_model/parsed_model
model_t model_init(uint8_t * data, vec3_t scale) {
    uint32_t j = 0;
    uint8_t num_frames = data[j++];
    uint8_t num_vertices = data[j++];
    uint8_t num_indices = data[j++];
    float vertices[num_vertices * num_frames * 3];
    uint8_t indices[num_indices * 3];

    int index_increment = 0;

    int min_x = 16;
    int max_x = -16;
    int min_y = 16;
    int max_y = -16;

    for (uint32_t i = 0; i < num_vertices * num_frames * 3; i += 3) {
        vertices[i] = (data[j++] - 15) * scale.x;
        vertices[i+1] = (data[j++] - 15) * scale.y;
        vertices[i+2] = (data[j++] - 15) * scale.z;

        // Find min/max only for the first frame
        if (i < num_vertices * 3) {
            min_x = min(min_x, vertices[i]);
            max_x = max(max_x, vertices[i]);
            min_y = min(min_y, vertices[i+1]);
            max_y = max(max_y, vertices[i+1]);
        }
    }

    // Load indices, 1x 2bit increment, 2x 7bit absolute
    for (uint32_t i = 0; i < num_indices * 3; i += 3) {
        index_increment += data[j++];
        indices[i] = index_increment;
        indices[i+1] = data[j++];
        indices[i+2] = data[j++];
    }

    // UV coords in texture space and width/height as fraction of model size
    float uf = 1.0f / (float)(max_x - min_x);
    float u = -min_x * uf;
    float vf = -1.0f / (float)(max_y - min_y);
    float v = max_y * vf;

    model_t out = {0};
    // todo, junk
    out.data = data;
    out.nv = num_indices * 3;

    for (uint32_t frame_index = 0; frame_index < num_frames; frame_index++) {
        out.frame_len++;
        out.frames = realloc(out.frames, sizeof(uint32_t) * out.frame_len);
        // todo, some weirdass alternative to pointers??
        out.frames[out.frame_len - 1] = r_num_verts;

        uint32_t vertex_offset = frame_index * num_vertices * 3;
        for (uint32_t i = 0; i < num_indices * 3; i += 3) {

            vec3_t mv[3];

            typedef struct {
                float u;
                float v;
            } uv_t;

            uv_t uv[3];
            for (uint32_t face_vertex = 0; face_vertex < 3; face_vertex++) {
                uint32_t idx = indices[i + face_vertex] * 3;
                mv[face_vertex] = vec3(
                                      vertices[vertex_offset + idx + 0],
                                      vertices[vertex_offset + idx + 1],
                                      vertices[vertex_offset + idx + 2]
                                  );
                uv[face_vertex] = (uv_t) {
                    .u = vertices[idx + 0] * uf + u,
                    .v = vertices[idx + 1] * vf + v
                };
            }

            vec3_t n = vec3_face_normal(mv[2], mv[1], mv[0]);
            r_push_vert(mv[2], n, uv[2].u, uv[2].v);
            r_push_vert(mv[1], n, uv[1].u, uv[1].v);
            r_push_vert(mv[0], n, uv[0].u, uv[0].v);
        }
    }

    return out;
}