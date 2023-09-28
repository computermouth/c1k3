
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "game.h"
#include "map.h"
#include "data.h"
#include "model.h"
#include "render.h"
#include "entity.h"
#include "entity_player.h"
#include "entity_enemy_grunt.h"
#include "entity_enemy_mutant.h"
#include "entity_enemy_enforcer.h"
#include "entity_enemy_ogre.h"
#include "entity_enemy_zombie.h"
#include "entity_enemy_hound.h"
#include "entity_pickup_nailgun.h"
#include "entity_pickup_grenadelauncher.h"
#include "entity_pickup_health.h"
#include "entity_pickup_nails.h"
#include "entity_pickup_grenades.h"
#include "entity_barrel.h"
#include "entity_light.h"
#include "entity_trigger_level.h"
#include "entity_door.h"
#include "entity_pickup_key.h"
#include "entity_torch.h"

#include "mpack.h"
#include "vector.h"

map_t * map;
vector * map_data = NULL;

typedef struct {
    char * entity_string;
    void (*constructor_func)(entity_t *, vec3_t, uint8_t, uint8_t);
} entity_table_t;

entity_table_t entity_table[__ENTITY_ID_END] = { 0 };

void map_init() {
    // populate entity_table for map parsing
    entity_table[ENTITY_ID_PLAYER] = (entity_table_t) {
        "player", entity_player_constructor
    };
    entity_table[ENTITY_ID_ENEMY_GRUNT] = (entity_table_t) {
        "grunt", entity_enemy_grunt_constructor
    };
    entity_table[ENTITY_ID_ENEMY_MUTANT] = (entity_table_t) {
        "mutant", entity_enemy_mutant_constructor
    };
    entity_table[ENTITY_ID_ENEMY_ENFORCER] = (entity_table_t) {
        "enforcer", entity_enemy_enforcer_constructor,
    };
    entity_table[ENTITY_ID_ENEMY_OGRE] = (entity_table_t) {
        "ogre", entity_enemy_ogre_constructor
    };
    entity_table[ENTITY_ID_ENEMY_ZOMBIE] = (entity_table_t) {
        "zombie", entity_enemy_zombie_constructor
    };
    entity_table[ENTITY_ID_ENEMY_HOUND] = (entity_table_t) {
        "hound", entity_enemy_hound_constructor
    };
    entity_table[ENTITY_ID_PICKUP_NAILGUN] = (entity_table_t) {
        "nailgun", entity_pickup_nailgun_constructor
    };
    entity_table[ENTITY_ID_PICKUP_SHOTGUN] = (entity_table_t) {
        "shotgun", NULL
    };
    entity_table[ENTITY_ID_PICKUP_GRENADELAUNCHER] = (entity_table_t) {
        "grenadelauncher", entity_pickup_grenadelauncher_constructor
    };
    entity_table[ENTITY_ID_PICKUP_HEALTH] = (entity_table_t) {
        "health", entity_pickup_health_constructor
    };
    entity_table[ENTITY_ID_PICKUP_NAILS] = (entity_table_t) {
        "nails", entity_pickup_nails_constructor
    };
    entity_table[ENTITY_ID_PICKUP_GRENADES] = (entity_table_t) {
        "key", entity_pickup_key_constructor
    };
    entity_table[ENTITY_ID_PICKUP_KEY] = (entity_table_t) {
        "grenades", entity_pickup_grenades_constructor
    };
    entity_table[ENTITY_ID_BARREL] = (entity_table_t) {
        "barrel", entity_barrel_constructor
    };
    entity_table[ENTITY_ID_LIGHT] = (entity_table_t) {
        "light", entity_light_constructor
    };
    entity_table[ENTITY_ID_TRIGGER_LEVEL] = (entity_table_t) {
        "trigger_levelchange", entity_trigger_level_constructor
    };
    entity_table[ENTITY_ID_DOOR] = (entity_table_t) {
        "door", entity_door_constructor
    };
    entity_table[ENTITY_ID_TORCH] = (entity_table_t) {
        "torch", entity_torch_constructor
    };
}

// todo, remove once levels have been reproduced
void (*spawn_class[])(entity_t *, vec3_t, uint8_t, uint8_t) = {
    /* 00 */ entity_player_constructor,
    /* 01 */ entity_enemy_grunt_constructor,
    /* 02 */ entity_enemy_enforcer_constructor,
    /* 03 */ entity_enemy_ogre_constructor,
    /* 04 */ entity_enemy_zombie_constructor,
    /* 05 */ entity_enemy_hound_constructor,
    /* 06 */ entity_pickup_nailgun_constructor,
    /* 07 */ entity_pickup_grenadelauncher_constructor,
    /* 08 */ entity_pickup_health_constructor,
    /* 09 */ entity_pickup_nails_constructor,
    /* 10 */ entity_pickup_grenades_constructor,
    /* 11 */ entity_barrel_constructor,
    /* 12 */ entity_light_constructor,
    /* 13 */ entity_trigger_level_constructor,
    /* 14 */ entity_door_constructor,
    /* 15 */ entity_pickup_key_constructor,
    /* 16 */ entity_torch_constructor,
};

ref_entt_t * map_ref_entt_from_name(char * name){
    size_t relen = vector_size(map->ref_entities);
    for(size_t i = 0; i < relen; i++) {
        ref_entt_t * re = vector_at(map->ref_entities, i);
        // the fuck?
        // ^because these can be zeroed until all constructors are ng
        if(re->name_len == 0)
            continue;
        if (strncmp(name, re->entity_name, re->name_len) == 0)
            return re;
    }
    
    return NULL;
}

uint32_t map_lookup_entity(const char * type_name, size_t len) {
    for(uint32_t i = 0; i < __ENTITY_ID_END; i++) {
        if (strncmp(type_name, entity_table[i].entity_string, len) == 0)
            return i;
    }
    fprintf(stderr, "E: failed to id entity %s -- skipping\n", type_name);
    return __ENTITY_ID_END;
}

vector * map_get_entity_kv(mpack_node_t n) {
    // will be nil if empty
    if (mpack_node_type(n) != mpack_type_map)
        return NULL;

    size_t nodelen = mpack_node_map_count(n);
    if (nodelen < 1)
        return NULL;

    vector * ep = vector_init(sizeof(entity_extra_params_t));
    for(size_t i = 0; i < nodelen; i++) {
        mpack_node_t k = mpack_node_map_key_at(n, i);
        mpack_node_t v = mpack_node_map_value_at(n, i);
        if(mpack_node_type(k) != mpack_type_str || mpack_node_type(v) != mpack_type_str)
            return NULL;
        size_t klen = mpack_node_strlen(k);
        size_t vlen = mpack_node_strlen(v);
        entity_extra_params_t tmpx = { 0 };
        strncpy(tmpx.k, mpack_node_str(k), mpack_node_strlen(k));
        strncpy(tmpx.v, mpack_node_str(v), mpack_node_strlen(v));
        vector_push(ep, &tmpx);
    }

    return ep;
}

void mpack_map_parse() {

    // pass in via args
    // const char * data = (char *)data_map3;
    // const size_t data_len = data_map3_len;

    const char * data = (char *)data_map1;
    const size_t data_len = data_map1_len;

    // push empty map, get pointer back
    map_t * tmp_map = vector_push(map_data, &(map_t) {
        0
    });

    mpack_tree_t tree = { 0 };
    mpack_tree_init_data(&tree, data, data_len);
    mpack_tree_parse(&tree);
    mpack_node_t root = mpack_tree_root(&tree);

    // todo, error handling, check if nil node
    mpack_node_t mp_ref_cubes = mpack_node_map_cstr(root, "ref_cubes");
    mpack_node_t mp_ref_entts = mpack_node_map_cstr(root, "ref_entts");
    mpack_node_t mp_map_cubes = mpack_node_map_cstr(root, "map_cubes");
    mpack_node_t mp_map_entts = mpack_node_map_cstr(root, "map_entts");

    size_t ref_cubes_sz = mpack_node_array_length(mp_ref_cubes);
    size_t ref_entts_sz = mpack_node_array_length(mp_ref_entts);
    size_t map_cubes_sz = mpack_node_array_length(mp_map_cubes);
    size_t map_entts_sz = mpack_node_array_length(mp_map_entts);

    // fprintf(stderr,
    //         "ref_cubes_sz: %zu\n"
    //         "ref_entts_sz: %zu\n"
    //         "map_cubes_sz: %zu\n"
    //         "map_entts_sz: %zu\n"
    //         ref_cubes_sz,
    //         ref_entts_sz,
    //         map_cubes_sz,
    //         map_entts_sz,
    //        );

    // populate ref cubes
    vector * ref_cube_tex_id_list = vector_init(sizeof(uint32_t));
    for(uint32_t i = 0; i < ref_cubes_sz; i++) {
        mpack_node_t bin = mpack_node_array_at(mp_ref_cubes, i);
        uint32_t id = r_create_texture((png_bin_t) {
            .data = (uint8_t *)mpack_node_bin_data(bin), .len = mpack_node_bin_size(bin)
        });
        // fprintf(stderr, "rcti [%u][%u]\n", i, id);
        vector_push(ref_cube_tex_id_list, &id);
    }

    // populate ref entts
    vector * ref_entt_list = vector_init(sizeof(ref_entt_t));
    for(uint32_t i = 0; i < __ENTITY_ID_END; i++)
        vector_push(ref_entt_list, &(ref_entt_t) {
        0
    });

    for(uint32_t i = 0; i < ref_entts_sz; i++) {
        ref_entt_t tmp_entt = { 0 };
        mpack_node_t entt_map = mpack_node_array_at(mp_ref_entts, i);

        // name
        mpack_node_t name_node = mpack_node_map_cstr(entt_map, "name");
        size_t name_len = mpack_node_strlen(name_node);
        const char * name = mpack_node_str(name_node);
        tmp_entt.name_len = name_len;
        strncpy(tmp_entt.entity_name, name, name_len);
        // fprintf(stderr, "packing %s\n", tmp_entt.entity_name);

        // texture
        mpack_node_t tex_bin = mpack_node_map_cstr(entt_map, "txtr");
        uint32_t tid = r_create_texture((png_bin_t) {
            .data = (uint8_t *)mpack_node_bin_data(tex_bin), .len = mpack_node_bin_size(tex_bin)
        });
        tmp_entt.tex_id = tid;

        // frame_names
        mpack_node_t frame_name_arr = mpack_node_map_cstr(entt_map, "anim_names");
        size_t frame_name_len = mpack_node_array_length(frame_name_arr);
        char (*fn)[frame_name_len][100] = calloc(1, sizeof(*fn));
        for(size_t fi = 0; fi < frame_name_len; fi++) {
            mpack_node_t fnas = mpack_node_array_at(frame_name_arr, fi);
            const char * fns = mpack_node_str(fnas);
            size_t fns_len = mpack_node_strlen(fnas);
            strncpy((*fn)[fi], fns, fns_len);
        }
        tmp_entt.frame_names = fn;

        // vert_len -- todo, check this better
        mpack_node_t u_arr = mpack_node_map_cstr(entt_map, "u");
        mpack_node_t v_arr = mpack_node_map_cstr(entt_map, "v");
        size_t ulen = mpack_node_array_length(u_arr);
        size_t vlen = mpack_node_array_length(v_arr);
        if (ulen != vlen)
            fprintf(stderr, "E: ulen != vlen on ref_entt[%d]\n", i);
        tmp_entt.vert_len = ulen;

        // u
        float * u = calloc(tmp_entt.vert_len, sizeof(float));
        for(size_t ui = 0; ui < tmp_entt.vert_len; ui++)
            u[ui] = mpack_node_float(mpack_node_array_at(u_arr, ui));
        // v
        float * v = calloc(tmp_entt.vert_len, sizeof(float));
        for(size_t vi = 0; vi < tmp_entt.vert_len; vi++)
            v[vi] = mpack_node_float(mpack_node_array_at(v_arr, vi));

        // frames
        mpack_node_t frame_arr = mpack_node_map_cstr(entt_map, "anim_frames");
        size_t frame_arr_len = mpack_node_array_length(frame_arr);
        if (frame_arr_len != frame_name_len)
            fprintf(stderr, "E: frame_arr_len != frame_name_len on ref_entt[%d]\n", i);
        float (*frames)[frame_arr_len][tmp_entt.vert_len][3] = calloc(1, sizeof(*frames));
        for(size_t fi = 0; fi < frame_arr_len; fi++) {
            mpack_node_t frame_vert_arr = mpack_node_array_at(frame_arr, fi);
            size_t fva_len = mpack_node_array_length(frame_vert_arr);
            for (size_t vi = 0; vi < tmp_entt.vert_len; vi++) {
                mpack_node_t frame_vert_xyz_arr = mpack_node_array_at(frame_vert_arr, vi);
                size_t fvax_len = mpack_node_array_length(frame_vert_xyz_arr);
                (*frames)[fi][vi][0] = mpack_node_float(mpack_node_array_at(frame_vert_xyz_arr, 0));
                (*frames)[fi][vi][1] = mpack_node_float(mpack_node_array_at(frame_vert_xyz_arr, 1));
                (*frames)[fi][vi][2] = mpack_node_float(mpack_node_array_at(frame_vert_xyz_arr, 2));
            }
        }
        tmp_entt.frame_len = frame_arr_len;
        tmp_entt.frames = model_load_ng(frames, frame_arr_len, ulen, u, v);
        free(u);
        free(v);
        free(frames);

        // insert tmp_entt into ref_entt_list[ENTITY_ID_X]
        entity_id_t id = map_lookup_entity(name, name_len);
        if(id == __ENTITY_ID_END) {
            continue;
        }
        vector_set(ref_entt_list, id, &tmp_entt);
    }
    tmp_map->ref_entities = ref_entt_list;

    // parse and store blocks + collision map
    vector * blocks = vector_init(sizeof(block_t));
    for(uint32_t i = 0; i < map_cubes_sz; i++) {
        mpack_node_t mp_mc = mpack_node_array_at(mp_map_cubes, i);

        mpack_node_t mp_mc_st = mpack_node_map_cstr(mp_mc, "start");
        mpack_node_t mp_mc_st_x = mpack_node_array_at(mp_mc_st, 0);
        mpack_node_t mp_mc_st_y = mpack_node_array_at(mp_mc_st, 1);
        mpack_node_t mp_mc_st_z = mpack_node_array_at(mp_mc_st, 2);

        mpack_node_t mp_mc_sz = mpack_node_map_cstr(mp_mc, "size");
        mpack_node_t mp_mc_sz_x = mpack_node_array_at(mp_mc_sz, 0);
        mpack_node_t mp_mc_sz_y = mpack_node_array_at(mp_mc_sz, 1);
        mpack_node_t mp_mc_sz_z = mpack_node_array_at(mp_mc_sz, 2);

        mpack_node_t mp_mc_id = mpack_node_map_cstr(mp_mc, "tex_id");

        uint64_t x = mpack_node_u64(mp_mc_st_x);
        uint64_t y = mpack_node_u64(mp_mc_st_y);
        uint64_t z = mpack_node_u64(mp_mc_st_z);
        uint64_t sx = mpack_node_u64(mp_mc_sz_x);
        uint64_t sy = mpack_node_u64(mp_mc_sz_y);
        uint64_t sz = mpack_node_u64(mp_mc_sz_z);
        uint64_t tex_index = mpack_node_u8(mp_mc_id);

        uint32_t global_tex_id = *(uint32_t *)vector_at(ref_cube_tex_id_list, tex_index);

        // todo, all cube geometry for the whole game is in vram
        // regardless of what level you're on?
        vector_push(blocks, &(block_t) {
            .t = global_tex_id,
            .b = r_push_block(x << 5, y << 4, z << 5, sx << 5, sy << 4, sz << 5, global_tex_id),
        });

        // todo, dynamic allocate 3d cm
        // and i guess round up to next multiple of 8
        // assign to pointer in tmp_map

        // The collision map is a bitmap; 8 x blocks per byte
        for (uint32_t cz = z; cz < z + sz; cz++) {
            for (uint32_t cy = y; cy < y + sy; cy++) {
                for (uint32_t cx = x; cx < x + sx; cx++) {
                    tmp_map->cm[
                        (
                            cz * map_size * map_size +
                            cy * map_size +
                            cx
                        ) >> 3
                    ] |= 1 << (cx & 7);
                }
            }
        }
    }
    tmp_map->blocks = blocks;

    vector * map_entities = vector_init(sizeof(entity_params_t));
    // push all entities
    for (uint32_t i = 0; i < map_entts_sz; i++) {
        mpack_node_t mp_me = mpack_node_array_at(mp_map_entts, i);
        mpack_node_t mp_me_type = mpack_node_map_cstr(mp_me, "type");
        mpack_node_t mp_me_pos = mpack_node_map_cstr(mp_me, "pos");
        
        size_t type_len = mpack_node_strlen(mp_me_type);
        const char * type_name = mpack_node_str(mp_me_type);
        entity_id_t id = map_lookup_entity(type_name, type_len);
        if(id == __ENTITY_ID_END) {
            fprintf(stderr, "E: failed to id entity %s -- skipping\n", type_name);
            continue;
        }

        // have to do these before the switch
        // not all will be present, so init has to be checked first

        // light-only param
        mpack_node_t mp_ml_rgba = { 0 };
        if(mpack_node_map_contains_cstr(mp_me, "color"))
            mp_ml_rgba = mpack_node_map_cstr(mp_me, "color");
        // normal-entt-only param
        mpack_node_t mp_ml_tex_id = { 0 };
        if(mpack_node_map_contains_cstr(mp_me, "tex_id"))
            mp_ml_tex_id = mpack_node_map_cstr(mp_me, "tex_id");
        // normal-entt-only param
        mpack_node_t mp_ml_extras = { 0 };
        if(mpack_node_map_contains_cstr(mp_me, "param"))
            mp_ml_extras = mpack_node_map_cstr(mp_me, "param");

        switch (id) {
        case ENTITY_ID_PLAYER:
            vector_push(map_entities, &(entity_params_t) {
                .id = id,
                .entity_player_params.position = {
                    .x = mpack_node_float(mpack_node_array_at(mp_me_pos, 0)),
                    .y = mpack_node_float(mpack_node_array_at(mp_me_pos, 1)),
                    .z = mpack_node_float(mpack_node_array_at(mp_me_pos, 2)),
                },
            });
            break;
        case ENTITY_ID_LIGHT:
            vector_push(map_entities, &(entity_params_t) {
                .id = id,
                .entity_light_params.position = {
                    .x = mpack_node_float(mpack_node_array_at(mp_me_pos, 0)),
                    .y = mpack_node_float(mpack_node_array_at(mp_me_pos, 1)),
                    .z = mpack_node_float(mpack_node_array_at(mp_me_pos, 2)),
                },
                .entity_light_params.rgba = {
                    mpack_node_u8(mpack_node_array_at(mp_ml_rgba, 0)),
                    mpack_node_u8(mpack_node_array_at(mp_ml_rgba, 1)),
                    mpack_node_u8(mpack_node_array_at(mp_ml_rgba, 2)),
                    mpack_node_u8(mpack_node_array_at(mp_ml_rgba, 3)),
                }
            });
            break;
        default:
            vector_push(map_entities, &(entity_params_t) {
                .id = id,
                .entity_generic_params.ref_entt = vector_at(tmp_map->ref_entities, id),
                .entity_generic_params.position = {
                    .x = mpack_node_float(mpack_node_array_at(mp_me_pos, 0)),
                    .y = mpack_node_float(mpack_node_array_at(mp_me_pos, 1)),
                    .z = mpack_node_float(mpack_node_array_at(mp_me_pos, 2)),
                },
                .entity_generic_params.extras = map_get_entity_kv(mp_ml_extras)
            });
        }
    }

    tmp_map->e_size = vector_size(map_entities);
    tmp_map->map_entities = map_entities;

    mpack_error_t err = mpack_tree_destroy(&tree);
    if ( err != mpack_ok) {
        fprintf(stderr, "An error occurred decoding the data -- %s!\n", mpack_error_to_string(err));
        return;
    }

    vector_free(ref_cube_tex_id_list);
}

void map_parse() {

    map_data = vector_init(sizeof(map_t));

    mpack_map_parse();

    const uint8_t * data = data_maps;

    for(uint32_t i = 0; i < data_maps_len;) {

        // push empty map, get pointer back
        map_t * tmp_map = vector_push(map_data, &(map_t) {
            0
        });

        uint32_t blocks_size = data[i] | (data[i+1] << 8);
        i += 2;
        // uint32_t blocks_size = data[i++] | (data[i++] << 8);
        uint8_t * b = (uint8_t *)&data[i];
        i += blocks_size;
        uint8_t cm[((map_size * map_size * map_size) >> 3) * sizeof(uint8_t)] = {0};

        vector * blocks = vector_init(sizeof(block_t));
        uint32_t t = 0;

        for(uint32_t j = 0; j < blocks_size;) {
            if(b[j] == 255) {
                j++;
                t = b[j++];
            }
            uint32_t x = b[j++];
            uint32_t y = b[j++];
            uint32_t z = b[j++];
            uint32_t sx = b[j++];
            uint32_t sy = b[j++];
            uint32_t sz = b[j++];

            vector_push(blocks, &(block_t) {
                .t = t,
                .b = r_push_block(x << 5, y << 4, z << 5, sx << 5, sy << 4, sz << 5, t),
            });

            // The collision map is a bitmap; 8 x blocks per byte
            for (uint32_t cz = z; cz < z + sz; cz++) {
                for (uint32_t cy = y; cy < y + sy; cy++) {
                    for (uint32_t cx = x; cx < x + sx; cx++) {
                        cm[
                            (
                                cz * map_size * map_size +
                                cy * map_size +
                                cx
                            ) >> 3
                        ] |= 1 << (cx & 7);
                    }
                }
            }
        }
        // Slice of entity data; we parse it when we actually spawn
        // the entities in map_init()
        uint32_t num_entities = data[i] | (data[i+1] << 8);
        i += 2;
        // uint32_t num_entities = data[i++] | (data[i++] << 8);
        uint8_t * e = (uint8_t *)(data + i);
        uint32_t e_size = num_entities * 6;
        i += e_size;

        // set map
        memcpy(tmp_map->cm, cm, ((map_size * map_size * map_size) >> 3) * sizeof(uint8_t));
        tmp_map->e = e;
        tmp_map->e_size = e_size;
        tmp_map->blocks = blocks;
    }

}

void map_load (map_t * m) {
    // todo, should this just be an index into a global map_collection_t?
    map = m;

    if (map->map_entities != NULL)
        goto map_load_ng;
    for (uint32_t i = 0; i < map->e_size;) {
        void (*func)(entity_t *, vec3_t, uint8_t, uint8_t) = spawn_class[map->e[i++]];
        int x = m->e[i++] * 32;
        int y = m->e[i++] * 16;
        int z = m->e[i++] * 32;
        uint8_t p1 = m->e[i++];
        uint8_t p2 = m->e[i++];
        if (func == NULL)
            continue;
        game_spawn(
            func,
        (vec3_t) {
            .x = x, .y = y, .z = z
        },
        p1,
        p2,
        NULL
        );
    }
    // exit if old maps were used
    return;

map_load_ng:
    for (uint32_t i = 0; i < map->e_size; i++) {
        entity_params_t * ep = vector_at(map->map_entities, i);
        void (*func)(entity_t *, vec3_t, uint8_t, uint8_t) = entity_table[ep->id].constructor_func;
        // todo, spawn the things with parameters
        if(ep->id == __ENTITY_ID_END) {
            fprintf(stderr, "E: unimp\n");
            continue;
        }
        if(ep->id == ENTITY_ID_LIGHT) {
            game_spawn(
                func,
            (vec3_t) {
                .x = ep->entity_light_params.position.x * 32,
                .y = ep->entity_light_params.position.y * 16,
                .z = ep->entity_light_params.position.z * 32
            }, 0, 0, ep);
        } else if(ep->id == ENTITY_ID_PLAYER) {
            game_spawn(
                func,
            (vec3_t) {
                .x = ep->entity_player_params.position.x * 32,
                .y = ep->entity_player_params.position.y * 16,
                .z = ep->entity_player_params.position.z * 32
            }, 0, 0, ep);
        } else {
            game_spawn(
                func,
            (vec3_t) {
                .x = ep->entity_generic_params.position.x * 32,
                .y = ep->entity_generic_params.position.y * 16,
                .z = ep->entity_generic_params.position.z * 32
            }, 0, 0, ep);
            // todo
            // free kv
        }
    }

}

uint8_t map_block_at(int32_t x, int32_t y, int32_t z) {
    if (x < 0 || y < 0 || z < 0) {
        fprintf(stderr, "E: tested collision in negative space\n");
        return 1;
    }
    return map->cm[
               (
                   z * map_size * map_size +
                   y * map_size +
                   x
               ) >> 3
        ] & (1 << (x & 7));
}

// todo, investigate if this should modify a
// it shouldn't, return bool, and don't modify a or b
bool map_trace(vec3_t a, vec3_t b) {
    vec3_t diff = vec3_sub(b, a);
    vec3_t step_dir = vec3_mulf(vec3_normalize(diff), 16);
    float steps = vec3_length(diff) / 16;

    for (uint32_t i = 0; i < steps; i++) {
        a = vec3_add(a, step_dir);
        if (map_block_at((int32_t)(a.x) >> 5, (int32_t)(a.y) >> 4, (int32_t)(a.z) >> 5)) {
            return true;
        }
    }

    return false;
}

int map_block_at_box(vec3_t box_start, vec3_t box_end) {
    for (int32_t z = (int32_t)(box_start.z) >> 5; z <= (int32_t)(box_end.z) >> 5; z++) {
        for (int32_t y = (int32_t)(box_start.y) >> 4; y <= (int32_t)(box_end.y) >> 4; y++) {
            for (int32_t x = (int32_t)(box_start.x) >> 5; x <= (int32_t)(box_end.x) >> 5; x++) {
                if (map_block_at(x, y, z)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void map_draw() {
    // todo, ??
    // previously was passed as pos for draw_call
    // vec3_t p = {0.0f, 0.0f, 0.0f};
    vector * blocks = map->blocks;
    uint32_t len = vector_size(blocks);
    for (uint32_t i = 0; i < len; i++) {

        block_t * block = vector_at(blocks, i);
        draw_call_t call = {
            .pos = {0.0f, 0.0f, 0.0f},
            .yaw = 0.0f,
            .pitch = 0.0f,
            .texture = block->t,
            .f1 = block->b,
            .f2 = block->b,
            .mix = 0.0f,
            .num_verts = 36
        };

        r_draw(call);
    }
}

typedef struct {
    vector * blocks;
    uint8_t * e;
    // map_entities and ref_entities is for new map format only
    vector * map_entities; // entity_params_t
    vector * ref_entities; // ref_entt_t
    vector * ref_ent_index; // size_t
    uint32_t e_size;
    uint8_t cm[((map_size * map_size * map_size) >> 3)];
} map_ts;

void map_quit() {
    uint32_t len = vector_size(map_data);
    for(uint32_t i = 0; i < len; i++) {
        map_t * map = vector_at(map_data, i);
        vector_free(map->blocks);

        if (map->map_entities) {
            size_t melen = vector_size(map->map_entities);
            for(size_t j = 0; j < melen; j++) {
                entity_params_t * ep = vector_at(map->map_entities, j);
                vector_free(ep->entity_generic_params.extras);
            }
            vector_free(map->map_entities);
        }

        if (map->ref_entities) {
            size_t relen = vector_size(map->ref_entities);
            for(size_t j = 0; j < relen; j++) {
                ref_entt_t * re = vector_at(map->ref_entities, j);
                free(re->frame_names);
                vector_free(re->frames);
            }
            vector_free(map->ref_entities);
        }
    }
    vector_free(map_data);
}
