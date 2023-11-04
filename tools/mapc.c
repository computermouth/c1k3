#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <libgen.h>

#include "vector.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include "mpack.h"

#include "mapc.h"

#define JSON_MAX_TOKENS 64

vector * ref_cube_vec = NULL;
vector * ref_entt_vec = NULL;

vector * map_cube_vec = NULL;
vector * map_entt_vec = NULL;

vector * out_cube_vec = NULL;
vector * out_entt_vec = NULL;
vector * out_lite_vec = NULL;
vector * out_plyr_vec = NULL;
vector * out_trgr_vec = NULL;

// turn this into
// POSITION
// TEXCOORD_0 (UV)
// abstraction

char * default_frame_name = "default";

typedef enum {
    ATN_TEXCOORD_0,
    ATN_POSITION,
} attname_t;

char * attstr[] = {
    "TEXCOORD_0",
    "POSITION"
};

cgltf_size index_of_attribute_name(cgltf_attribute * attributes, cgltf_size att_len, attname_t a){
    char * astr = attstr[a];
    for(cgltf_size i = 0; i < att_len; i++){
        if (strcmp(attributes[i].name, astr) == 0)
            return i;
    }
    
    fprintf(stderr, "E: couldn't find attstr[%d]: %s\n", a, astr);
    exit(1);
    return 9999;
}

mapc_verts_t get_verts_from_mesh(cgltf_mesh * mesh) {

    mapc_verts_t out_v = {0};

    // push target names
    out_v.anim_names = vector_init(sizeof(char *));
    vector_push(out_v.anim_names, &default_frame_name);
    for(size_t i = 0; i < mesh->target_names_count; i++) {
        fprintf(stderr, "anim_name: %s\n", mesh->target_names[i]);
        vector_push(out_v.anim_names, &(mesh->target_names[i]));
    }

    size_t pc = mesh->primitives_count;
    if (mesh->primitives_count > 1)
        fprintf(stderr, "W: '%s' primitives > 1 -- %zu", mesh->name, pc);
    cgltf_primitive prim = mesh->primitives[0];

    // Find the index of the POSITION, TEXCOORD_0(uv) attributes, and the index accessor
    cgltf_accessor * position_accessor = prim.attributes[index_of_attribute_name(prim.attributes, prim.attributes_count, ATN_POSITION)].data;
    cgltf_accessor * uv_accessor = prim.attributes[index_of_attribute_name(prim.attributes, prim.attributes_count, ATN_TEXCOORD_0)].data;
    cgltf_accessor * index_accessor = prim.indices;

    if (!position_accessor || !uv_accessor || !index_accessor) {
        fprintf(stderr, "E: missing position, uv, or index attribute/accessor\n");
        return out_v;
    }
    
    // get all indices
    size_t index_count = index_accessor->count;
    uint16_t (*indices)[index_count] = calloc(sizeof(*indices), 1);
    for(size_t i = 0; i < index_count; i++){
        cgltf_uint t;
        cgltf_bool success = cgltf_accessor_read_uint(index_accessor, i, &t, 1);
        (*indices)[i] = t;
        if (!success) exit(1);
    }
    out_v.index_count = index_count;
    
    // get all positions
    float (*pos)[position_accessor->count][3] = calloc(sizeof(*pos), 1);
    for(size_t i = 0; i < position_accessor->count; i++){
        cgltf_bool success = cgltf_accessor_read_float(position_accessor, i, (cgltf_float*)&(*pos)[i], 3);
        if (!success) exit(1);
    }

    // get all uvs
    float (*uvs)[uv_accessor->count][2] = calloc(sizeof(*uvs), 1);
    for(size_t i = 0; i < uv_accessor->count; i++) {
        cgltf_bool success = cgltf_accessor_read_float(uv_accessor, i, (cgltf_float*)&(*uvs)[i], 2);
        if (!success) exit(1);
    }
    
    // parse uvs out per-face
    out_v.u = vector_init(sizeof(float));
    out_v.v = vector_init(sizeof(float));
    for(size_t i = 0; i < index_count; i++){
        uint16_t index = (*indices)[i];
        float * uv = (*uvs)[index];
        vector_push(out_v.u, &uv[0]);
        vector_push(out_v.v, &uv[1]);
    }

    // push default frame
    out_v.anim_frames = vector_init(sizeof(mapc_fpos3_t) * index_count);
    mapc_fpos3_t * tmp_frame = malloc(sizeof(mapc_fpos3_t) * index_count);
    for(size_t i = 0; i < index_count; i++) {
        uint16_t index = (*indices)[i];
        float * tmp_pos = (*pos)[index];
        tmp_frame[i].x = tmp_pos[0];
        tmp_frame[i].y = tmp_pos[1];
        tmp_frame[i].z = tmp_pos[2];
    }
    vector_push(out_v.anim_frames, tmp_frame);
    mapc_fpos3_t * default_frame = vector_at(out_v.anim_frames, 0);
    
    // create and push target frames
    for(size_t i = 0; i < prim.targets_count; i++) {
        cgltf_morph_target* morph_target = &(prim.targets[i]);
        cgltf_accessor * morph_position_accessor = morph_target->attributes[index_of_attribute_name(morph_target->attributes, morph_target->attributes_count, ATN_POSITION)].data;
        
        // get all morph positions
        float (*m_pos)[morph_position_accessor->count][3] = calloc(sizeof(*m_pos), 1);
        for(size_t j = 0; j < morph_position_accessor->count; j++) {
            cgltf_bool success = cgltf_accessor_read_float(morph_position_accessor, j, (cgltf_float*)&(*m_pos)[j], 3);
            if (!success) exit(1);
        }

        for(size_t j = 0; j < index_count; j++) {
            uint16_t index = (*indices)[j];
            float * tmp_pos = (*m_pos)[index];
            tmp_frame[j].x += tmp_pos[0];
            tmp_frame[j].y += tmp_pos[1];
            tmp_frame[j].z += tmp_pos[2];
            // fprintf(stderr, "i: %zu -- {%f, %f, %f}\n", j, tmp_frame[j].x, tmp_frame[j].y, tmp_frame[j].z);
        }
        vector_push(out_v.anim_frames, tmp_frame);
        // reset tmp_frame to default
        memcpy(tmp_frame, default_frame, sizeof(mapc_fpos3_t) * index_count);
        free(m_pos);
    }
    free(indices);
    free(pos);
    free(uvs);
    free(tmp_frame);

    return out_v;
}

mapc_txtr_t get_image(cgltf_node* node) {

    if (node->mesh == NULL) {
        fprintf(stderr, "Node does not have a mesh.\n");
        exit(1);
        return (mapc_txtr_t) {
            0
        };
    }

    cgltf_mesh * mesh = node->mesh;
    if (mesh->primitives == NULL) {
        fprintf(stderr, "Mesh does not have primitives.\n");
        exit(1);
        return (mapc_txtr_t) {
            0
        };
    }

    cgltf_primitive prim = mesh->primitives[0];
    if (prim.material == NULL) {
        fprintf(stderr, "Primitive does not have a material.\n");
        exit(1);
        return (mapc_txtr_t) {
            0
        };
    }

    cgltf_material * material = prim.material;
    cgltf_pbr_metallic_roughness* pbr = &material->pbr_metallic_roughness;

    if (pbr->base_color_texture.texture == NULL) {
        fprintf(stderr, "Material does not have an associated texture.\n");
        exit(1);
        return (mapc_txtr_t) {
            0
        };
    }

    cgltf_texture* texture_view = pbr->base_color_texture.texture;

    if (texture_view->image == NULL) {
        fprintf(stderr, "Texture does not have an associated image.\n");
        exit(1);
        return (mapc_txtr_t) {
            0
        };
    }

    cgltf_image* image = texture_view->image;

    if (image->uri != NULL && strlen(image->uri) > 0) {
        fprintf(stderr, "URI-based images are not supported in this example.\n");
        exit(1);
        return (mapc_txtr_t) {
            0
        };
    }

    if (image->buffer_view == NULL) {
        fprintf(stderr, "Image does not have a buffer view.\n");
        exit(1);
        return (mapc_txtr_t) {
            0
        };
    }

    fprintf(stderr, "found image: %s.png\n", image->name);

    return (mapc_txtr_t) {
        .len = image->buffer_view->size,
        .data = (unsigned char*)image->buffer_view->buffer->data + image->buffer_view->offset
    };
}

int32_t find_cube_txtr_id(uint8_t * txtr_data) {
    size_t len = vector_size(ref_cube_vec);
    if(len > 255)
        fprintf(stderr, "W: found more than 255 cube textures!");
    for(size_t i = 0; i < len; i++) {
        mapc_rm_cube_t * ref_cube = vector_at(ref_cube_vec, i);
        if (ref_cube->txtr.data == txtr_data)
            return i;
    }
    fprintf(stderr, "null?: %p\n", txtr_data);
    return -1;
}

int32_t find_entt_txtr_id(uint8_t * txtr_data) {
    size_t len = vector_size(ref_entt_vec);
    for(size_t i = 0; i < len; i++) {
        mapc_rm_entt_t * ref_entt = vector_at(ref_entt_vec, i);
        if (ref_entt->txtr.data == txtr_data)
            return i;
    }
    return -1;
}

void prep_out() {

    size_t len = 0;

    // map cube -> out cube
    len = vector_size(map_cube_vec);
    for(size_t i = 0; i < len; i++) {
        mapc_rm_cube_t * cube = vector_at(map_cube_vec, i);
        // int32_t texture_id = 0;
        int32_t texture_id = find_cube_txtr_id(cube->txtr.data);
        // if (texture_id < 0 || texture_id > 254)
        //     continue;

        mapc_out_cube_t oc = {
            .start = {
                .x = (uint64_t)cube->start.x,
                .y = (uint64_t)cube->start.y,
                .z = (uint64_t)cube->start.z,
            },
            .size = { // todo, do some rounding?
                .x = (uint64_t)cube->size.x,
                .y = (uint64_t)cube->size.y,
                .z = (uint64_t)cube->size.z,
            },
            .texture = (uint8_t)texture_id,
        };

        vector_push(out_cube_vec, &oc);
    }
    vector_free(map_cube_vec);

    // map entt -> out entt
    len = vector_size(map_entt_vec);
    for(size_t i = 0; i < len; i++) {
        mapc_rm_entt_t * entt = vector_at(map_entt_vec, i);
        // int32_t texture_id = 0;
        int32_t texture_id = find_entt_txtr_id(entt->txtr.data);
        // if (texture_id < 0 || texture_id > 254)
        //     continue;

        mapc_out_entt_t oe = {
            .fpos = {
                .x = entt->fpos.x,
                .y = entt->fpos.y,
                .z = entt->fpos.z,
            },
            .texture = (uint8_t)texture_id,
            .extras = entt->extras,
        };
        strncpy(oe.entity_name, entt->entity_name, 100);

        vector_push(out_entt_vec, &oe);
    }
    vector_free(map_entt_vec);

    fprintf(stderr, "outcubelen: %zu\n", vector_size(out_cube_vec));
    fprintf(stderr, "outenttlen: %zu\n", vector_size(out_entt_vec));
    fprintf(stderr, "outlitelen: %zu\n", vector_size(out_lite_vec));
    fprintf(stderr, "outplyrlen: %zu\n", vector_size(out_plyr_vec));

}

bool node_is_cube(cgltf_node * node) {
    if(node->extras.data == NULL)
        return false;

    jsmn_parser parser = { 0 };
    jsmntok_t tokens[JSON_MAX_TOKENS] = { 0 };
    const char * xjson = node->extras.data;

    jsmn_init(&parser);
    int count = jsmn_parse(&parser, xjson, strlen(xjson), tokens, JSON_MAX_TOKENS);

    for(int i = 0; i < count; i++) {
        if( tokens[i].type == JSMN_STRING
                && tokens[i].size > 0
                && strncmp(xjson + tokens[i].start, "_type", tokens[i].end - tokens[i].start) == 0
                && tokens[i + 1].type == JSMN_STRING
                && strncmp(xjson + tokens[i + 1].start, "cube", tokens[i + 1].end - tokens[i + 1].start) == 0
          )
            return true;
    }

    return false;
}

bool node_is_entity(cgltf_node * node, char * name) {
    if(node->extras.data == NULL)
        return false;

    jsmn_parser parser = { 0 };
    jsmntok_t tokens[JSON_MAX_TOKENS] = { 0 };
    const char * xjson = node->extras.data;

    jsmn_init(&parser);
    int count = jsmn_parse(&parser, xjson, strlen(xjson), tokens, JSON_MAX_TOKENS);

    bool is_entity = false;
    for(int i = 0; i < count; i++) {
        if( tokens[i].type == JSMN_STRING
                && tokens[i].size > 0
                && strncmp(xjson + tokens[i].start, "_type", tokens[i].end - tokens[i].start) == 0
                && tokens[i + 1].type == JSMN_STRING
                && strncmp(xjson + tokens[i + 1].start, "entity", tokens[i + 1].end - tokens[i + 1].start) == 0
          ){
            is_entity = true;
          }
    }
    
    if (!is_entity)
        return false;
    
    for(int i = 0; i < count; i++) {
        if( tokens[i].type == JSMN_STRING
                && tokens[i].size > 0
                && strncmp(xjson + tokens[i].start, "_entity", tokens[i].end - tokens[i].start) == 0
                && tokens[i + 1].type == JSMN_STRING
          ){
            strncpy(name, xjson + tokens[i + 1].start, tokens[i + 1].end - tokens[i + 1].start);
            name[tokens[i + 1].end - tokens[i + 1].start] = '\0';
            fprintf(stderr, "type: %s\n", name);
            return true;
          }
    }
    
    fprintf(stderr, "E: couldn't find entity_name");

    return false;
}

bool entity_is_player(cgltf_node * node) {
    if(node->extras.data == NULL)
        return false;

    jsmn_parser parser = { 0 };
    jsmntok_t tokens[JSON_MAX_TOKENS] = { 0 };
    const char * xjson = node->extras.data;

    jsmn_init(&parser);
    int count = jsmn_parse(&parser, xjson, strlen(xjson), tokens, JSON_MAX_TOKENS);

    for(int i = 0; i < count; i++) {
        if( tokens[i].type == JSMN_STRING
                && tokens[i].size > 0
                && strncmp(xjson + tokens[i].start, "_entity", tokens[i].end - tokens[i].start) == 0
                && tokens[i + 1].type == JSMN_STRING
                && strncmp(xjson + tokens[i + 1].start, "player", tokens[i + 1].end - tokens[i + 1].start) == 0
          )
            return true;
    }

    return false;
}

bool entity_is_light(cgltf_node * node) {
    if(node->extras.data == NULL)
        return false;

    jsmn_parser parser = { 0 };
    jsmntok_t tokens[JSON_MAX_TOKENS] = { 0 };
    const char * xjson = node->extras.data;

    jsmn_init(&parser);
    int count = jsmn_parse(&parser, xjson, strlen(xjson), tokens, JSON_MAX_TOKENS);

    for(int i = 0; i < count; i++) {
        if( tokens[i].type == JSMN_STRING
                && tokens[i].size > 0
                && strncmp(xjson + tokens[i].start, "_entity", tokens[i].end - tokens[i].start) == 0
                && tokens[i + 1].type == JSMN_STRING
                && strncmp(xjson + tokens[i + 1].start, "light", tokens[i + 1].end - tokens[i + 1].start) == 0
          )
            return true;
    }

    return false;
}

bool entity_is_trigger(cgltf_node * node) {
    
    if(node->extras.data == NULL)
        return false;

    jsmn_parser parser = { 0 };
    jsmntok_t tokens[JSON_MAX_TOKENS] = { 0 };
    const char * xjson = node->extras.data;

    jsmn_init(&parser);
    int count = jsmn_parse(&parser, xjson, strlen(xjson), tokens, JSON_MAX_TOKENS);
    
    for(int i = 0; i < count; i++) {
        if( tokens[i].type == JSMN_STRING
                && tokens[i].size > 0
                && strncmp(xjson + tokens[i].start, "_entity", tokens[i].end - tokens[i].start) == 0
                && tokens[i + 1].type == JSMN_STRING
                && strncmp(xjson + tokens[i + 1].start, "trigger_", strlen("trigger_")) == 0
          )
            return true;
    }

    return false;
}

bool light_from_node(cgltf_node * node, mapc_out_lite_t * out) {

    if(node->extras.data == NULL)
        goto fail;

    jsmn_parser parser = { 0 };
    jsmntok_t tokens[JSON_MAX_TOKENS] = { 0 };
    const char * xjson = node->extras.data;

    jsmn_init(&parser);
    int count = jsmn_parse(&parser, xjson, strlen(xjson), tokens, JSON_MAX_TOKENS);

    for(int i = 0; i < count; i++) {

        if( tokens[i].type == JSMN_STRING
                && tokens[i].size > 0
                && strncmp(xjson + tokens[i].start, "color", tokens[i].end - tokens[i].start) == 0
                && tokens[i + 1].size == 4
          ) {
            out->color[0] = atoi(xjson + tokens[i + 2 + 0].start);
            out->color[1] = atoi(xjson + tokens[i + 2 + 1].start);
            out->color[2] = atoi(xjson + tokens[i + 2 + 2].start);
            out->color[3] = atoi(xjson + tokens[i + 2 + 3].start);

            return true;
        }
    }

fail:
    fprintf(stderr, "E: failed to get color of lights");
    exit(1);
}

vector * extras_strings_from_node(cgltf_node * node){
    if (node->extras.data == NULL)
        return NULL;
    
    vector * kv_vec = vector_init(sizeof(mapc_extra_kv_t));

    jsmn_parser parser = { 0 };
    jsmntok_t tokens[JSON_MAX_TOKENS] = { 0 };
    const char * xjson = node->extras.data;

    jsmn_init(&parser);
    int count = jsmn_parse(&parser, xjson, strlen(xjson), tokens, JSON_MAX_TOKENS);

    for(int i = 0; i < count; i++) {
        if( tokens[i].type == JSMN_STRING
                && tokens[i].size > 0
                && strncmp(xjson + tokens[i].start, "_entity", tokens[i].end - tokens[i].start) != 0
                && strncmp(xjson + tokens[i].start, "_type", tokens[i].end - tokens[i].start) != 0
                && tokens[i + 1].type == JSMN_STRING
          ){
                mapc_extra_kv_t tmp_kv = {0};
                strncpy(tmp_kv.k, xjson + tokens[i].start, tokens[i].end - tokens[i].start);
                strncpy(tmp_kv.v, xjson + tokens[i + 1].start, tokens[i + 1].end - tokens[i + 1].start);
              fprintf(stderr, "found kv: {'%s': '%s'}\n", tmp_kv.k, tmp_kv.v);
              vector_push(kv_vec, &tmp_kv);
          }
    }
    
    // extras was not empty
    if (vector_size(kv_vec))
        return kv_vec;
    // extras was empty
    vector_free(kv_vec);
    return NULL;
}

typedef enum {
    CLASS_REF,
    CLASS_MAP,
} mesh_class_t;

typedef enum {
    GROUP_INVALID,
    GROUP_CUBE,
    GROUP_ENTT,
    GROUP_ENTT_TRIGGER,
    GROUP_ENTT_LIGHT,
    GROUP_ENTT_PLAYER,
} mesh_group_t;

mapc_pos3_t group_meshes(cgltf_data * data) {
    mapc_pos3_t max_xyz = { 0, 0, 0 };

    // todo, skip entities
    for (size_t i = 0; i < data->nodes_count; ++i) {

        cgltf_node * n = &(data->nodes[i]);
        mesh_group_t mg = GROUP_INVALID;
        mesh_class_t mc = CLASS_MAP;
        mapc_out_lite_t ol = { 0 };
        char entity_name[100];

        // determine group
        if(node_is_cube(n))
            mg = GROUP_CUBE;
        else if (node_is_entity(n, entity_name)) {
            if(entity_is_light(n))
                mg = GROUP_ENTT_LIGHT;
            else if(entity_is_player(n))
                mg = GROUP_ENTT_PLAYER;
            else if(entity_is_trigger(n))
                mg = GROUP_ENTT_TRIGGER;
            else
                mg = GROUP_ENTT;
        }

        mapc_fpos3_t start = {
            .x = n->translation[0],
            .y = n->translation[1],
            .z = n->translation[2] * 2,
        };

        mapc_fpos3_t size = {
            .x = n->scale[0],
            .y = n->scale[1],
            .z = n->scale[2] * 2,
        };

        // todo, if cube, start and end are either .0f or .5f

        if (size.x < 0 || size.y < 0 || size.z < 0) {
            fprintf(stderr, "W: scale was negative, skipping '%s'\n", n->name);
            continue;
        }

        mapc_fpos3_t end = {
            .x = start.x + size.x,
            .y = start.y + size.y,
            .z = start.z + size.z,
        };

        if (
            start.x < 0 || end.x < 0 ||
            start.y < 0 || end.y < 0 ||
            start.z < 0 || end.z < 0
        ) {
            mc = CLASS_REF;
            goto skip_negative;
        }

        // find the largest index in the mesh, and check against max
        if (end.x > max_xyz.x) max_xyz.x = end.x;
        if (end.y > max_xyz.y) max_xyz.y = end.y;
        if (end.z > max_xyz.z) max_xyz.z = end.z;

        // one or more indicies was in negative space
skip_negative:

        switch (mc) {
        case CLASS_REF:
            switch (mg) {
            case GROUP_CUBE:
                // ref cube
                vector_push(ref_cube_vec, &(mapc_rm_cube_t) {
                    .txtr = get_image(n), .start = start, .size = size
                });
                fprintf(stderr, "ref_cube_vec '%s'\n", n->name);
                break;
            case GROUP_ENTT:
                // ref entt
                vector_push(ref_entt_vec, &(mapc_rm_entt_t) {
                    .txtr = get_image(n), .fpos = start, .verts = get_verts_from_mesh(n->mesh)
                });
                strncpy(((mapc_rm_entt_t *)vector_at(ref_entt_vec, vector_size(ref_entt_vec) - 1))->entity_name, entity_name, 100);
                fprintf(stderr, "ref_entt_vec '%s'\n", n->name);
                break;
            case GROUP_ENTT_TRIGGER:
                // ref trigger -> discard
                fprintf(stderr, "ref_trgr_light - discarding '%s'\n", n->name);
                break;
            case GROUP_ENTT_LIGHT:
                // ref light -> discard
                fprintf(stderr, "ref_entt_light - discarding '%s'\n", n->name);
                break;
            case GROUP_ENTT_PLAYER:
                // ref player -> discard
                fprintf(stderr, "ref_entt_playr - discarding '%s'\n", n->name);
                break;
            default:
                fprintf(stderr, "invalid group for ref node[%lu]('%s')\n", i, n->name);
                exit(1);
                break;
            }
            break;
        case CLASS_MAP:
            switch (mg) {
            case GROUP_CUBE:
                // map cube
                vector_push(map_cube_vec, &(mapc_rm_cube_t) {
                    .txtr = get_image(n), .start = start, .size = size
                });
                fprintf(stderr, "map_cube_vec '%s'\n", n->name);
                break;
            case GROUP_ENTT:
                // map entt
                vector_push(map_entt_vec, &(mapc_rm_entt_t) {
                    .txtr = get_image(n), .fpos = start, .extras = extras_strings_from_node(n)
                });
                strncpy(((mapc_rm_entt_t *)vector_at(map_entt_vec, vector_size(map_entt_vec) - 1))->entity_name, entity_name, 100);
                fprintf(stderr, "map_entt_vec '%s'\n", n->name);
                break;
            case GROUP_ENTT_TRIGGER:
                // map trigger
                vector_push(out_trgr_vec, &(mapc_out_entt_t) {
                    .fpos = start, .extras = extras_strings_from_node(n)
                });
                strncpy(((mapc_out_entt_t *)vector_at(out_trgr_vec, vector_size(out_trgr_vec) - 1))->entity_name, entity_name, 100);
                fprintf(stderr, "map_trgr_vec '%s'\n", n->name);
                break;
            case GROUP_ENTT_LIGHT:
                ol.fpos = start;
                light_from_node(n, &ol);
                vector_push(out_lite_vec, &ol);
                fprintf(stderr, "map_entt_light '%s'\n", n->name);
                break;
            case GROUP_ENTT_PLAYER:
                // ref player -> discard
                vector_push(out_plyr_vec, &(mapc_out_plyr_t) {
                    .fpos = start
                });
                fprintf(stderr, "map_entt_playr '%s'\n", n->name);
                break;
            default:
                fprintf(stderr, "invalid group for map node[%lu]('%s')\n", i, n->name);
                exit(1);
                break;
            }
            break;
        default:
            fprintf(stderr, "invalid class for node[%lu]('%s')\n", i, n->name);
            exit(1);
            break;
        }

    }

    // check if the dimensions are too large
    // todo, test
    uint64_t rollover_xy = max_xyz.x * max_xyz.y;
    if (max_xyz.x != 0 && rollover_xy / max_xyz.x != max_xyz.y) {
        // overflowed
        fprintf(stderr, "map is too large to allocate at { %lu, %lu, %lu }", max_xyz.x, max_xyz.y, max_xyz.z);
        exit(1);
    }

    uint64_t rollover_z = rollover_xy * max_xyz.z;
    if (max_xyz.y != 0 && rollover_z / rollover_xy != max_xyz.z) {
        // overflowed
        fprintf(stderr, "map is too large to allocate at { %lu, %lu, %lu }", max_xyz.x, max_xyz.y, max_xyz.z);
        exit(1);
    }

    return max_xyz;
}

// 270 degrees in radians
#define DEG270 (3.0 * 3.14159 / 2.0)

mapc_fpos3_t rotate_on_x(mapc_fpos3_t fp_in){
    float rot_matrix[3][3] = {
        {cos(DEG270), -sin(DEG270), 0},
        {sin(DEG270), cos(DEG270), 0},
        {0, 0, 1}
    };
    
    float f_i[3] = {fp_in.x, fp_in.y, fp_in.z};
    float f_o[3] = { 0 };
    
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            f_o[i] += rot_matrix[i][j] * f_i[j];
    
    return (mapc_fpos3_t){.x = f_o[0], .y = f_o[1], .z = f_o[2]};
}

int main(int argc, char * argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s <file.gltf>\n", argv[0]);
        exit(1);
    }

    char * file_path = argv[1];
    // char * tar_path = argv[2];

    // Load the glTF file using cgltf
    cgltf_data* data;
    cgltf_options options = {0};
    cgltf_result result = cgltf_parse_file(&options, file_path, &data);

    if (result != cgltf_result_success) {
        fprintf(stderr, "Failed to parse glTF file: %s\n", file_path);
        return 1;
    }

    // Load the glTF file and access the meshes
    result = cgltf_load_buffers(&options, data, file_path);

    if (result != cgltf_result_success) {
        fprintf(stderr, "Failed to load buffers\n");
        return 1;
    }

    ref_cube_vec = vector_init(sizeof(mapc_rm_cube_t));
    map_cube_vec = vector_init(sizeof(mapc_rm_cube_t));

    ref_entt_vec = vector_init(sizeof(mapc_rm_entt_t));
    map_entt_vec = vector_init(sizeof(mapc_rm_entt_t));

    out_cube_vec = vector_init(sizeof(mapc_out_cube_t));
    out_entt_vec = vector_init(sizeof(mapc_out_entt_t));
    out_lite_vec = vector_init(sizeof(mapc_out_lite_t));
    out_plyr_vec = vector_init(sizeof(mapc_out_plyr_t));
    out_trgr_vec = vector_init(sizeof(mapc_out_entt_t));

    prep_out();

    // mpack time
    // encode to memory buffer
    char* mp_data;
    size_t size;
    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, &mp_data, &size);

    // root map
    mpack_build_map(&writer);

    // cube references
    {
        mpack_write_cstr(&writer, "ref_cubes");
        size_t rclen = vector_size(ref_cube_vec);
        mpack_start_array(&writer, rclen);
        for(size_t i = 0; i < rclen; i++) {
            mapc_rm_cube_t * rc = vector_at(ref_cube_vec, i);
            mpack_start_bin(&writer, rc->txtr.len);
            mpack_write_bytes(&writer, (char *)rc->txtr.data, rc->txtr.len);
            mpack_finish_bin(&writer);
        }
        mpack_finish_array(&writer);
    }

    // entt references
    {
        mpack_write_cstr(&writer, "ref_entts");
        size_t ntlen = vector_size(ref_entt_vec);
        mpack_start_array(&writer, ntlen);
        for(size_t i = 0; i < ntlen; i++) {
            mpack_start_map(&writer, 6);
            mapc_rm_entt_t * re = vector_at(ref_entt_vec, i);
            {   // entity_name
                mpack_write_cstr(&writer, "name");
                mpack_write_cstr(&writer, re->entity_name);
            }
            
            {   // txtr
                mpack_write_cstr(&writer, "txtr");
                mpack_start_bin(&writer, re->txtr.len);
                mpack_write_bytes(&writer, (char *)re->txtr.data, re->txtr.len);
                mpack_finish_bin(&writer);
            }

            {   // u
                mpack_write_cstr(&writer, "u");
                size_t ulen = vector_size(re->verts.u);
                if (ulen != re->verts.index_count)
                    fprintf(stderr, "E: ref_entt[%zu] -- ulen(%zu) != index_len(%zu)\n", i, ulen, re->verts.index_count);
                mpack_start_array(&writer, ulen);
                for(size_t j = 0; j < ulen; j++)
                    mpack_write_float(&writer, *(float *)vector_at(re->verts.u, j));
                mpack_finish_array(&writer);
            }

            {   // v
                mpack_write_cstr(&writer, "v");
                size_t vlen = vector_size(re->verts.v);
                if (vlen != re->verts.index_count)
                    fprintf(stderr, "E: ref_entt[%zu] -- vlen != index_len\n", i);
                mpack_start_array(&writer, vlen);
                for(size_t j = 0; j < vlen; j++)
                    mpack_write_float(&writer, *(float *)vector_at(re->verts.v, j));
                mpack_finish_array(&writer);
            }

            {   // anim_names
                mpack_write_cstr(&writer, "anim_names");
                size_t aname_len = vector_size(re->verts.anim_names);
                mpack_start_array(&writer, aname_len);
                for(size_t j = 0; j < aname_len; j++)
                    mpack_write_cstr(&writer, *(char **)vector_at(re->verts.anim_names, j));
                mpack_finish_array(&writer);
            }

            {   // anim_frames
                mpack_write_cstr(&writer, "anim_frames");
                size_t aframe_len = vector_size(re->verts.anim_frames);
                mpack_start_array(&writer, aframe_len);
                for(size_t j = 0; j < aframe_len; j++) {
                    mapc_fpos3_t * anim_frame = vector_at(re->verts.anim_frames,j);
                    mpack_start_array(&writer, re->verts.index_count);
                    for(size_t k = 0; k < re->verts.index_count; k++) {
                        mpack_start_array(&writer, 3);
                        // rotate 270 degrees on x, then y<->z swap                     
                        mapc_fpos3_t t = rotate_on_x(anim_frame[k]);
                        mpack_write_float(&writer, -t.x);
                        mpack_write_float(&writer, t.z);
                        mpack_write_float(&writer, t.y);
                        
                        mpack_finish_array(&writer);
                    }
                    mpack_finish_array(&writer);
                }
                mpack_finish_array(&writer);
            }

            mpack_finish_map(&writer);
        }
        mpack_finish_array(&writer);
    }

    // map cubes
    {
        mpack_write_cstr(&writer, "map_cubes");
        size_t mclen = vector_size(out_cube_vec);
        mpack_start_array(&writer, mclen);
        for(size_t i = 0; i < mclen; i++) {
            mapc_out_cube_t * oc = vector_at(out_cube_vec, i);
            mpack_start_map(&writer, 3);

            mpack_write_cstr(&writer, "tex_id");
            mpack_write_u8(&writer, oc->texture);

            mpack_write_cstr(&writer, "start");
            mpack_start_array(&writer, 3);
            mpack_write_u64(&writer, oc->start.x);
            mpack_write_u64(&writer, oc->start.z);
            mpack_write_u64(&writer, oc->start.y);
            mpack_finish_array(&writer);

            mpack_write_cstr(&writer, "size");
            mpack_start_array(&writer, 3);
            mpack_write_u64(&writer, oc->size.x);
            mpack_write_u64(&writer, oc->size.z);
            mpack_write_u64(&writer, oc->size.y);
            mpack_finish_array(&writer);

            mpack_finish_map(&writer);
        }
        mpack_finish_array(&writer);
    }

    // map entts
    {
        mpack_write_cstr(&writer, "map_entts");
        size_t arrlen = vector_size(out_entt_vec) + vector_size(out_lite_vec) + vector_size(out_plyr_vec) + vector_size(out_trgr_vec);
        // size_t arrlen = vector_size(out_entt_vec);
        mpack_start_array(&writer, arrlen);
        size_t melen = vector_size(out_entt_vec);
        for(size_t i = 0; i < melen; i++) {
            mapc_out_entt_t * oe = vector_at(out_entt_vec, i);
            mpack_start_map(&writer, 4);

            mpack_write_cstr(&writer, "type");
            mpack_write_cstr(&writer, oe->entity_name);

            mpack_write_cstr(&writer, "tex_id");
            mpack_write_u8(&writer, oe->texture);

            mpack_write_cstr(&writer, "pos");
            mpack_start_array(&writer, 3);
            mpack_write_float(&writer, oe->fpos.x);
            mpack_write_float(&writer, oe->fpos.z);
            mpack_write_float(&writer, oe->fpos.y);
            mpack_finish_array(&writer);
            
            mpack_write_cstr(&writer, "param");
            if(oe->extras){
                size_t param_len = vector_size(oe->extras);
                mpack_start_map(&writer, param_len);
                for(size_t i = 0; i < param_len; i++){
                    mpack_write_cstr(&writer, ((mapc_extra_kv_t*)vector_at(oe->extras, i))->k);
                    mpack_write_cstr(&writer, ((mapc_extra_kv_t*)vector_at(oe->extras, i))->v);
                }
                mpack_finish_map(&writer);
            } else {
                mpack_write_nil(&writer);
            }

            mpack_finish_map(&writer);
        }
        // map lights
        {
            size_t ollen = vector_size(out_lite_vec);
            for(size_t i = 0; i < ollen; i++) {
                mapc_out_lite_t * ol = vector_at(out_lite_vec, i);
                mpack_start_map(&writer, 3);
    
                mpack_write_cstr(&writer, "type");
                mpack_write_cstr(&writer, "light");
    
                mpack_write_cstr(&writer, "color");
                mpack_start_array(&writer, 4);
                mpack_write_u8(&writer, ol->color[0]);
                mpack_write_u8(&writer, ol->color[1]);
                mpack_write_u8(&writer, ol->color[2]);
                mpack_write_u8(&writer, ol->color[3]);
                mpack_finish_array(&writer);
    
                mpack_write_cstr(&writer, "pos");
                mpack_start_array(&writer, 3);
                mpack_write_float(&writer, ol->fpos.x);
                mpack_write_float(&writer, ol->fpos.z);
                mpack_write_float(&writer, ol->fpos.y);
                mpack_finish_array(&writer);
    
                mpack_finish_map(&writer);
            }
        }
        // map player
        {
            size_t pllen = vector_size(out_plyr_vec);
            if (pllen != 1)
                fprintf(stderr, "E: len(player) == %zu\n", pllen);
    
            mapc_out_plyr_t * op = vector_at(out_plyr_vec, 0);
            mpack_start_map(&writer, 2);
            mpack_write_cstr(&writer, "type");
            mpack_write_cstr(&writer, "player");
            
            mpack_write_cstr(&writer, "pos");
            mpack_start_array(&writer, 3);
            mpack_write_float(&writer, op->fpos.x);
            mpack_write_float(&writer, op->fpos.z);
            mpack_write_float(&writer, op->fpos.y);
            mpack_finish_array(&writer);
            mpack_finish_map(&writer);
        }
        // map triggers
        {
            size_t trlen = vector_size(out_trgr_vec);
            for(size_t i = 0; i < trlen; i++) {
                mapc_out_entt_t * ot = vector_at(out_trgr_vec, i);
                mpack_start_map(&writer, 3);
    
                mpack_write_cstr(&writer, "type");
                mpack_write_cstr(&writer, ot->entity_name);
    
                mpack_write_cstr(&writer, "pos");
                mpack_start_array(&writer, 3);
                mpack_write_float(&writer, ot->fpos.x);
                mpack_write_float(&writer, ot->fpos.z);
                mpack_write_float(&writer, ot->fpos.y);
                mpack_finish_array(&writer);
            
                mpack_write_cstr(&writer, "param");
                if(ot->extras){
                    size_t param_len = vector_size(ot->extras);
                    mpack_start_map(&writer, param_len);
                    for(size_t i = 0; i < param_len; i++){
                        mpack_write_cstr(&writer, ((mapc_extra_kv_t*)vector_at(ot->extras, i))->k);
                        mpack_write_cstr(&writer, ((mapc_extra_kv_t*)vector_at(ot->extras, i))->v);
                    }
                    mpack_finish_map(&writer);
                } else {
                    mpack_write_nil(&writer);
                }
    
                mpack_finish_map(&writer);
            }
        }
        
        mpack_finish_array(&writer);
    }

    mpack_complete_map(&writer);

    // finish writing
    if (mpack_writer_destroy(&writer) != mpack_ok) {
        fprintf(stderr, "An error occurred encoding the data!\n");
    }

    // fwrite some shit
    fwrite(mp_data, sizeof(char), size, stdout);

    free(mp_data);

    vector_free(ref_cube_vec);

    size_t len = vector_size(ref_entt_vec);
    for(size_t i = 0; i < len; i++) {
        mapc_rm_entt_t * e = vector_at(ref_entt_vec, i);
        vector_free(e->verts.u);
        vector_free(e->verts.v);
        vector_free(e->verts.anim_names);
        vector_free(e->verts.anim_frames);
    }
    vector_free(ref_entt_vec);

    vector_free(out_cube_vec);

    len = vector_size(out_entt_vec);
    for(size_t i = 0; i < len; i++) {
        mapc_out_entt_t * e = vector_at(out_entt_vec, i);
        vector_free(e->extras);
    }
    vector_free(out_entt_vec);
    vector_free(out_lite_vec);
    vector_free(out_plyr_vec);

    len = vector_size(out_trgr_vec);
    for(size_t i = 0; i < len; i++) {
        mapc_out_entt_t * e = vector_at(out_trgr_vec, i);
        vector_free(e->extras);
    }
    vector_free(out_trgr_vec);

    cgltf_free(data);

   return 0;
}
