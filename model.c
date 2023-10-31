
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "model.h"
#include "data.h"
#include "render.h"
#include "vector.h"

// todo, return different type, like entity_model/parsed_model
model_t model_load(uint8_t * data, vec3_t scale) {
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

vector * model_load_ng(void * void_verts, uint32_t frame_len, uint32_t vert_len, float * u, float * v) {
    // todo, scale??
    vector * frame_ids = vector_init(sizeof(uint32_t));
    float (*model_verts)[frame_len][vert_len][3] = void_verts;

    uint32_t u_i = 0;
    uint32_t v_i = 0;
    // frame
    for (uint32_t frame_i = 0; frame_i < frame_len; frame_i++) {
        // point to first vertex in frame
        vector_push(frame_ids, &r_num_verts);

        // reset uv indices
        u_i = 0;
        v_i = 0;
        // vertex of frame
        for (uint32_t vert_i = 0; vert_i < vert_len; vert_i+=3) {

            vec3_t fv[3];

            typedef struct {
                float u;
                float v;
            } uv_t;
            uv_t uv[3];

            for (uint32_t face_i = 0; face_i < 3; face_i++) {

                // scale to match cube dimensions
                fv[face_i] = vec3(
                                 // flip x because blender
                                 (*model_verts)[frame_i][vert_i + face_i][0] * -32,
                                 (*model_verts)[frame_i][vert_i + face_i][1] * 32,
                                 (*model_verts)[frame_i][vert_i + face_i][2] * 32
                             );
                uv[face_i] = (uv_t) {
                    .u = u[u_i++], // god forgive me
                    .v = v[v_i++]  // maybe could be vert_i / 3 + face_i, fuck
                };
            }

            // switch vertex winding for normals and verts because blender
            vec3_t n = vec3_face_normal(fv[0], fv[1], fv[2]);
            r_push_vert(fv[0], n, uv[0].u, uv[0].v);
            r_push_vert(fv[1], n, uv[1].u, uv[1].v);
            r_push_vert(fv[2], n, uv[2].u, uv[2].v);
        }
    }

    return frame_ids;
}

void model_quit() {
    // todo, do these in map_quit once everything's sorted
}