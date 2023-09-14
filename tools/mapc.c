#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <libgen.h>

#include "vector.h"
// #include "microtar.h"
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

// turn this into
// POSITION
// TEXCOORD_0 (UV)
// abstraction

char * default_frame_name = "default";

mapc_verts_t get_verts_from_mesh(cgltf_mesh * mesh) {

    mapc_verts_t out_v = {0};

    // Find the index of the TEXCOORD_0 attribute
    cgltf_attribute* uvAttribute = NULL;
    for (cgltf_size i = 0; i < mesh->primitives[0].attributes_count; ++i) {
        if (strcmp(mesh->primitives[0].attributes[i].name, "TEXCOORD_0") == 0) {
            uvAttribute = &mesh->primitives[0].attributes[i];
            break;
        }
    }

    // Find the index of the POSITION attribute
    cgltf_attribute* position_attribute = NULL;
    for (cgltf_size i = 0; i < mesh->primitives[0].attributes_count; ++i) {
        if (strcmp(mesh->primitives[0].attributes[i].name, "POSITION") == 0) {
            position_attribute = &mesh->primitives[0].attributes[i];
            break;
        }
    }

    if (!uvAttribute || !position_attribute) {
        fprintf(stderr, "E: missing uv or position attribute\n");
        return out_v;
    }

    cgltf_accessor* uvAccessor = uvAttribute->data;
    cgltf_buffer_view* uvBufferView = uvAccessor->buffer_view;
    cgltf_buffer* uvBuffer = uvBufferView->buffer;

    cgltf_accessor* position_accessor = position_attribute->data;

    if(position_accessor->count != uvAccessor->count) {
        fprintf(stderr, "E: position count doesn't match uv count\n");
        return out_v;
    }

    // get uv values
    size_t uvCount = uvAccessor->count;
    size_t uvOffset = uvAccessor->offset + uvBufferView->offset;
    float* uvData = (float*)(uvBuffer->data + uvOffset);

    // get position
    for (size_t i = 0; i < mesh->target_names_count; ++i) {
        fprintf(stderr, "morph_target: %s\n", mesh->target_names[i]);
    }

    // done
    out_v.u = vector_init(sizeof(float));
    out_v.v = vector_init(sizeof(float));
    for(size_t i = 0; i < uvCount; i++) {
        vector_push(out_v.u, &(uvData[i * 2 + 0]));
        vector_push(out_v.v, &(uvData[i * 2 + 1]));
    }

    // push target names
    out_v.anim_names = vector_init(sizeof(char *));
    vector_push(out_v.anim_names, &default_frame_name);
    for(size_t i = 0; i < mesh->target_names_count; i++) {
        fprintf(stderr, "anim_name: %s\n", mesh->target_names[i]);
        vector_push(out_v.anim_names, &(mesh->target_names[i]));
    }

    // push default frame
    out_v.anim_frames = vector_init(sizeof(mapc_fpos3_t) * uvCount);
    mapc_fpos3_t * tmp_frame = malloc(sizeof(mapc_fpos3_t) * uvCount);
    for(size_t i = 0; i < position_accessor->count; i++) {
        cgltf_float tmp[3];
        cgltf_accessor_read_float(position_accessor, i, tmp, 3);
        tmp_frame[i].x = tmp[0];
        tmp_frame[i].y = tmp[1];
        tmp_frame[i].z = tmp[2];
    }
    vector_push(out_v.anim_frames, tmp_frame);
    mapc_fpos3_t * default_frame = vector_at(out_v.anim_frames, 0);

    // create and push target frames
    cgltf_primitive * tmp_prim = &(mesh->primitives[0]);
    for(size_t i = 0; i < tmp_prim->targets_count; i++) {
        cgltf_morph_target* morphTarget = &(tmp_prim->targets[i]);
        // ought to check if attributes[0] is POSITION
        cgltf_attribute* morphPositionAttr = &morphTarget->attributes[0];
        fprintf(stderr, "name: %s\n", morphPositionAttr->name);

        cgltf_accessor* morph_position_accessor = morphPositionAttr->data;
        cgltf_buffer_view* morph_position_buffer_view = morph_position_accessor->buffer_view;
        cgltf_buffer* morph_position_buffer = morph_position_buffer_view->buffer;

        for(size_t j = 0; j < morph_position_accessor->count; j++) {
            cgltf_float tmp[3];
            bool ok = cgltf_accessor_read_float(morph_position_accessor, j, tmp, 3);
            tmp_frame[j].x += tmp[0];
            tmp_frame[j].y += tmp[1];
            tmp_frame[j].z += tmp[2];
        }
        vector_push(out_v.anim_frames, tmp_frame);
        // reset tmp_frame to default
        memcpy(tmp_frame, default_frame, sizeof(mapc_fpos3_t) * uvCount);
    }
    free(tmp_frame);

    return out_v;
}

mapc_txtr_t get_image(cgltf_node* node) {

    if (node->mesh == NULL) {
        fprintf(stderr, "Node does not have a mesh.\n");
        return (mapc_txtr_t) {
            0
        };
    }

    cgltf_mesh * mesh = node->mesh;
    if (mesh->primitives == NULL) {
        fprintf(stderr, "Mesh does not have primitives.\n");
        return (mapc_txtr_t) {
            0
        };
    }

    cgltf_primitive prim = mesh->primitives[0];
    if (prim.material == NULL) {
        fprintf(stderr, "Primitive does not have a material.\n");
        return (mapc_txtr_t) {
            0
        };
    }

    cgltf_material * material = prim.material;
    cgltf_pbr_metallic_roughness* pbr = &material->pbr_metallic_roughness;

    if (pbr->base_color_texture.texture == NULL) {
        fprintf(stderr, "Material does not have an associated texture.\n");
        return (mapc_txtr_t) {
            0
        };
    }

    cgltf_texture* texture_view = pbr->base_color_texture.texture;

    if (texture_view->image == NULL) {
        fprintf(stderr, "Texture does not have an associated image.\n");
        return (mapc_txtr_t) {
            0
        };
    }

    cgltf_image* image = texture_view->image;

    if (image->uri != NULL && strlen(image->uri) > 0) {
        fprintf(stderr, "URI-based images are not supported in this example.\n");
        return (mapc_txtr_t) {
            0
        };
    }

    if (image->buffer_view == NULL) {
        fprintf(stderr, "Image does not have a buffer view.\n");
        return (mapc_txtr_t) {
            0
        };
    }

    fprintf(stderr, "found image: %s.png\n", image->name);

    return (mapc_txtr_t) {
        .len = image->buffer_view->size,
        .data = (unsigned char*)(image->buffer_view->buffer->data + image->buffer_view->offset)
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
        };

        vector_push(out_entt_vec, &oe);
    }

    fprintf(stderr, "outcubelen: %zu\n", vector_size(out_cube_vec));
    fprintf(stderr, "outenttlen: %zu\n", vector_size(out_entt_vec));
    fprintf(stderr, "outlitelen: %zu\n", vector_size(out_lite_vec));
    fprintf(stderr, "outplyrlen: %zu\n", vector_size(out_plyr_vec));

}
/*
void map_meshes_to_array(cgltf_data * data, pos3_t max, uint8_t (*arr)[max.x][max.y][max.z]) {

    fprintf(stderr, "nodes: %d\n", (int)data->nodes_count);

    for (size_t i = 0; i < data->nodes_count; ++i) {

        cgltf_node * n = &(data->nodes[i]);

        cgltf_float m[16] = {0};
        cgltf_node_transform_world(n, m);

        cgltf_mesh* mesh = n->mesh;

        // Calculate the dimensions of the primitive
        pos3_t mesh_min_xyz = {UINT64_MAX, UINT64_MAX, UINT64_MAX};
        pos3_t mesh_max_xyz = {0, 0, 0};

        if (mesh->primitives_count > 1)
            fprintf(stderr, "W: primitive_count > 1\n");

        uint8_t texture_id = 0;

        // Iterate over the primitives of the mesh
        for (size_t j = 0; j < mesh->primitives_count; ++j) {

            cgltf_primitive* primitive = &mesh->primitives[j];

            // if (primitive->material) {
            //     texture_id = get_material_index(primitive->material);
            // } else {
            //     fprintf(stderr, "W: primitive has no texture\n");
            //     continue;
            // }

            if (texture_id == 0) {
                fprintf(stderr, "W: texture_id is 0\n");
                continue;
            }

            // Access the positions of the primitive
            cgltf_accessor* position_accessor = primitive->attributes[0].data;
            cgltf_buffer_view* position_view = position_accessor->buffer_view;
            cgltf_buffer* position_buffer = position_view->buffer;
            float* positions = (float*)(position_buffer->data + position_view->offset + position_accessor->offset);

            size_t positionCount = position_accessor->count;
            size_t positionStride = position_accessor->stride / sizeof(float);

            // Apply the transformation matrix to each vertex position
            for (size_t k = 0; k < positionCount; ++k) {
                float x = positions[k * positionStride];
                float y = positions[k * positionStride + 1];
                float z = positions[k * positionStride + 2];

                // Apply the transformation matrix to the vertex position
                float transformedX = x * m[0] + y * m[4] + z * m[8] + m[12];
                float transformedY = x * m[1] + y * m[5] + z * m[9] + m[13];
                float transformedZ = x * m[2] + y * m[6] + z * m[10] + m[14];

                // Negative space, shouldn't be found
                if(transformedX < 0 && transformedY < 0 && transformedZ < 0){
                    fprintf(stderr, "W: encountered mesh('%s') in negative space {%ld, %ld, %ld}\n",
                        mesh->name, (int64_t)transformedX, (int64_t)transformedY, (int64_t)transformedZ);
                    vector_push(ref_cube_vec, &n);
                    goto skip_oob;
                }

                // Calculate the integer indices of the transformed vertex position
                uint64_t posX = (uint64_t)transformedX;
                uint64_t posY = (uint64_t)transformedY;
                uint64_t posZ = (uint64_t)transformedZ;

                // Update the minimum and maximum positions
                mesh_min_xyz.x = (uint64_t)fmin(mesh_min_xyz.x, posX);
                mesh_min_xyz.y = (uint64_t)fmin(mesh_min_xyz.y, posY);
                mesh_min_xyz.z = (uint64_t)fmin(mesh_min_xyz.z, posZ);

                mesh_max_xyz.x = (uint64_t)fmax(mesh_max_xyz.x, posX);
                mesh_max_xyz.y = (uint64_t)fmax(mesh_max_xyz.y, posY);
                mesh_max_xyz.z = (uint64_t)fmax(mesh_max_xyz.z, posZ);
            }
        }

        if( //mins
               mesh_min_xyz.x < 0 || mesh_min_xyz.x > max.x
            || mesh_min_xyz.y < 0 || mesh_min_xyz.y > max.y
            || mesh_min_xyz.z < 0 || mesh_min_xyz.z > max.z
            // maxes
            || mesh_max_xyz.x < 0 || mesh_max_xyz.x > max.x
            || mesh_max_xyz.y < 0 || mesh_max_xyz.y > max.y
            || mesh_max_xyz.z < 0 || mesh_max_xyz.z > max.z
        ){
            fprintf(stderr,
                "W: block { %lu, %lu, %lu } - { %lu, %lu, %lu } is outside range of { 0, 0, 0 } <-> { %lu, %lu, %lu } -- skipping\n -- (this should never be seen in practice)",
                mesh_min_xyz.x, mesh_min_xyz.y, mesh_min_xyz.z,
                mesh_max_xyz.x, mesh_max_xyz.y, mesh_max_xyz.z,
                max.x, max.y, max.z
            );
            goto skip_oob;
        }

        // old array packer
        // // Set the corresponding indices in the boolean map array
        // for (uint64_t x = mesh_min_xyz.x; x < mesh_max_xyz.x; ++x)
        //     for (uint64_t y = mesh_min_xyz.y; y < mesh_max_xyz.y; ++y)
        //         for (uint64_t z = mesh_min_xyz.z; z < mesh_max_xyz.z; ++z)
        //             (*arr)[x][y][z] = texture_id;

        // vector_push()

        fprintf(stderr, "mesh: %s\n", mesh->name);
        fprintf(stderr, "  - { %lu, %lu, %lu } - { %lu, %lu, %lu }\n",
             mesh_min_xyz.x, mesh_min_xyz.y, mesh_min_xyz.z,
             mesh_max_xyz.x, mesh_max_xyz.y, mesh_max_xyz.z
        );

        // one or more indicies was in negative space
        skip_oob:
            continue;
    }

    // Clean up cgltf resources
    // cgltf_free(data);
}
*/
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
                && strncmp(xjson + tokens[i].start, "type", tokens[i].end - tokens[i].start) == 0
                && tokens[i + 1].type == JSMN_STRING
                && strncmp(xjson + tokens[i + 1].start, "cube", tokens[i + 1].end - tokens[i + 1].start) == 0
          )
            return true;
    }

    return false;
}

bool node_is_entity(cgltf_node * node) {
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
                && strncmp(xjson + tokens[i].start, "type", tokens[i].end - tokens[i].start) == 0
                && tokens[i + 1].type == JSMN_STRING
                && strncmp(xjson + tokens[i + 1].start, "entity", tokens[i + 1].end - tokens[i + 1].start) == 0
          )
            return true;
    }

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
                && strncmp(xjson + tokens[i].start, "entity", tokens[i].end - tokens[i].start) == 0
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
                && strncmp(xjson + tokens[i].start, "entity", tokens[i].end - tokens[i].start) == 0
                && tokens[i + 1].type == JSMN_STRING
                && strncmp(xjson + tokens[i + 1].start, "light", tokens[i + 1].end - tokens[i + 1].start) == 0
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

typedef enum {
    CLASS_REF,
    CLASS_MAP,
} mesh_class_t;

typedef enum {
    GROUP_INVALID,
    GROUP_CUBE,
    GROUP_ENTT,
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

        // determine group
        if(node_is_cube(n))
            mg = GROUP_CUBE;
        else if (node_is_entity(n)) {
            mg = GROUP_ENTT;
            if(entity_is_light(n))
                mg = GROUP_ENTT_LIGHT;
            if(entity_is_player(n))
                mg = GROUP_ENTT_PLAYER;
        }

        mapc_fpos3_t start = {
            .x = n->translation[0],
            .y = n->translation[1],
            .z = n->translation[2],
        };

        mapc_fpos3_t size = {
            .x = n->scale[0],
            .y = n->scale[1],
            .z = n->scale[2],
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
                fprintf(stderr, "ref_entt_vec '%s'\n", n->name);
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
                    .txtr = get_image(n), .fpos = start
                });
                fprintf(stderr, "map_entt_vec '%s'\n", n->name);
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
                break;
            }
            break;
        default:
            fprintf(stderr, "invalid class for node[%lu]('%s')\n", i, n->name);
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

    mapc_pos3_t max_p = group_meshes(data);

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
            mpack_start_map(&writer, 5);
            mapc_rm_entt_t * re = vector_at(ref_entt_vec, i);
            {   // txtr
                mpack_write_cstr(&writer, "txtr");
                mpack_start_bin(&writer, re->txtr.len);
                mpack_write_bytes(&writer, (char *)re->txtr.data, re->txtr.len);
                mpack_finish_bin(&writer);
            }

            {   // u
                mpack_write_cstr(&writer, "u");
                size_t ulen = vector_size(re->verts.u);
                mpack_start_array(&writer, ulen);
                for(size_t j = 0; j < ulen; j++)
                    mpack_write_float(&writer, *(float *)vector_at(re->verts.u, j));
                mpack_finish_array(&writer);
            }

            {   // v
                mpack_write_cstr(&writer, "v");
                size_t vlen = vector_size(re->verts.v);
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

            size_t ulen = vector_size(re->verts.u);
            size_t vlen = vector_size(re->verts.v);
            if (ulen != vlen)
                break;

            {   // anim_frames
                mpack_write_cstr(&writer, "anim_frames");
                size_t aframe_len = vector_size(re->verts.anim_frames);
                mpack_start_array(&writer, aframe_len);
                for(size_t j = 0; j < aframe_len; j++) {
                    mapc_fpos3_t * anim_frame = vector_at(re->verts.anim_frames,j);
                    mpack_start_array(&writer, ulen);
                    for(size_t k = 0; k < ulen; k++) {
                        mpack_start_array(&writer, 3);
                        mpack_write_float(&writer, anim_frame[k].x);
                        mpack_write_float(&writer, anim_frame[k].y);
                        mpack_write_float(&writer, anim_frame[k].z);
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
            mpack_write_u64(&writer, oc->start.y);
            mpack_write_u64(&writer, oc->start.z);
            mpack_finish_array(&writer);

            mpack_write_cstr(&writer, "size");
            mpack_start_array(&writer, 3);
            mpack_write_u64(&writer, oc->size.x);
            mpack_write_u64(&writer, oc->size.y);
            mpack_write_u64(&writer, oc->size.z);
            mpack_finish_array(&writer);

            mpack_finish_map(&writer);
        }
        mpack_finish_array(&writer);
    }

    // map entts
    {
        mpack_write_cstr(&writer, "map_entts");
        size_t melen = vector_size(out_entt_vec);
        mpack_start_array(&writer, melen);
        for(size_t i = 0; i < melen; i++) {
            mapc_out_entt_t * oe = vector_at(out_entt_vec, i);
            mpack_start_map(&writer, 2);

            mpack_write_cstr(&writer, "tex_id");
            mpack_write_u8(&writer, oe->texture);

            mpack_write_cstr(&writer, "pos");
            mpack_start_array(&writer, 3);
            mpack_write_float(&writer, oe->fpos.x);
            mpack_write_float(&writer, oe->fpos.y);
            mpack_write_float(&writer, oe->fpos.z);
            mpack_finish_array(&writer);

            mpack_finish_map(&writer);
        }
        mpack_finish_array(&writer);
    }

    // map lights
    {
        mpack_write_cstr(&writer, "map_lites");
        size_t ollen = vector_size(out_lite_vec);
        mpack_start_array(&writer, ollen);
        for(size_t i = 0; i < ollen; i++) {
            mapc_out_lite_t * ol = vector_at(out_lite_vec, i);
            mpack_start_map(&writer, 2);

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
            mpack_write_float(&writer, ol->fpos.y);
            mpack_write_float(&writer, ol->fpos.z);
            mpack_finish_array(&writer);

            mpack_finish_map(&writer);
        }
        mpack_finish_array(&writer);
    }

    // map player
    {
        mpack_write_cstr(&writer, "map_player");
        size_t pllen = vector_size(out_plyr_vec);
        if (pllen != 1)
            fprintf(stderr, "E: len(player) == %zu\n", pllen);

        mapc_out_plyr_t * op = vector_at(out_plyr_vec, 0);

        mpack_start_array(&writer, 3);
        mpack_write_float(&writer, op->fpos.x);
        mpack_write_float(&writer, op->fpos.y);
        mpack_write_float(&writer, op->fpos.z);
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
    vector_free(map_cube_vec);

    size_t len = vector_size(ref_entt_vec);
    for(size_t i = 0; i < len; i++) {
        mapc_rm_entt_t * e = vector_at(ref_entt_vec, i);
        vector_free(e->verts.u);
        vector_free(e->verts.v);
        vector_free(e->verts.anim_names);
        vector_free(e->verts.anim_frames);
    }
    vector_free(ref_entt_vec);
    vector_free(map_entt_vec);

    vector_free(out_cube_vec);
    vector_free(out_entt_vec);

    cgltf_free(data);

    return 0;
}
