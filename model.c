
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "model.h"
#include "data.h"
#include "render.h"
#include "vector.h"

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