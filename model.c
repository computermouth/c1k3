
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "model.h"
#include "data.h"
#include "render.h"
#include "vector.h"

vector * model_data = NULL;
// todo, change all these to pointers,
// raise memory management to a higher level
// Q
model_t model_q = { 0 };
// Particles
model_t model_explosion = {0};
model_t model_blood = {0};
model_t model_gib = {0};
vector * model_gib_pieces = NULL;
// Enemies
model_t model_grunt = {0};
model_t model_enforcer = {0};
model_t model_ogre = {0};
model_t model_zombie = {0};
model_t model_hound = {0};
// Map Objects
model_t model_barrel = {0};
model_t model_torch = {0};
// Weapon view models
model_t model_shotgun = {0};
model_t model_nailgun = {0};
model_t model_grenadelauncher = {0};
// Pickups
model_t model_pickup_nailgun = {0};
model_t model_pickup_grenadelauncher = {0};
model_t model_pickup_box = {0};
model_t model_pickup_grenades = {0};
model_t model_pickup_key = {0};
model_t model_door = {0};
// Projectiles
model_t model_grenade = {0};
model_t model_nail = {0}; // aka. nail

void model_parse() {

    model_data = vector_init(sizeof(model_t));
    model_gib_pieces = vector_init(sizeof(model_t));

    const uint8_t * data = data_models;

    for (uint32_t i = 0; i < data_models_len;) {
        // let model_size = num_frames * num_verts * 3 + num_indices * 3
        uint32_t model_size = (data[i] * data[i+1] + data[i+2]) * 3;
        i += 3;
        vector_push(model_data, &(model_t) {
            .data = (uint8_t *)(data + (i - 3)),
            .data_len = model_size,
        });
        i += model_size;
    }
}

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

            vec3_t n = vec3_face_normal(fv[2], fv[1], fv[0]);
            
            // switch vertex winding because blender
            r_push_vert(fv[0], n, uv[0].u, uv[0].v);
            r_push_vert(fv[1], n, uv[1].u, uv[1].v);
            r_push_vert(fv[2], n, uv[2].u, uv[2].v);
        }
    }

    return frame_ids;
}

void model_quit() {

    // todo, do these in map_quit once everything's sorted
    if (model_q.frames)                      free(model_q.frames);
    if (model_explosion.frames)              free(model_explosion.frames);
    if (model_blood.frames)                  free(model_blood.frames);
    if (model_gib.frames)                    free(model_gib.frames);
    // if (model_grunt.frames)                  free(model_grunt.frames);
    if (model_enforcer.frames)               free(model_enforcer.frames);
    if (model_zombie.frames)                 free(model_zombie.frames);
    if (model_ogre.frames)                   free(model_ogre.frames);
    if (model_hound.frames)                  free(model_hound.frames);
    if (model_barrel.frames)                 free(model_barrel.frames);
    if (model_torch.frames)                  free(model_torch.frames);
    if (model_shotgun.frames)                free(model_shotgun.frames);
    if (model_nailgun.frames)                free(model_nailgun.frames);
    if (model_grenadelauncher.frames)        free(model_grenadelauncher.frames);
    if (model_pickup_nailgun.frames)         free(model_pickup_nailgun.frames);
    if (model_pickup_grenadelauncher.frames) free(model_pickup_grenadelauncher.frames);
    if (model_pickup_box.frames)             free(model_pickup_box.frames);
    if (model_pickup_grenades.frames)        free(model_pickup_grenades.frames);
    if (model_pickup_key.frames)             free(model_pickup_key.frames);
    if (model_door.frames)                   free(model_door.frames);
    if (model_grenade.frames)                free(model_grenade.frames);
    if (model_nail.frames)                   free(model_nail.frames);

    uint32_t len = vector_size(model_gib_pieces);
    for(uint32_t i = 0; i < len; i++) {
        model_t * m = vector_at(model_gib_pieces, i);
        free(m->frames);
    }
    vector_free(model_gib_pieces);

    vector_free(model_data);
}