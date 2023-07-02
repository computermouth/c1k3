
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "game.h"
#include "map.h"
#include "data.h"
#include "render.h"
#include "entity.h"
#include "entity_player.h"
#include "entity_enemy_grunt.h"
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

map_t * map;
map_collection_t map_data = { 0 };

map_collection_t map_parse() {

    const uint8_t * data = data_maps;
    map_t * tmp_maps = NULL;
    uint32_t tmp_maps_len = 0;
    for(uint32_t i = 0; i < data_maps_len;) {
        tmp_maps_len++;
        tmp_maps = realloc(tmp_maps, sizeof(map_t) * tmp_maps_len);

        uint32_t blocks_size = data[i] | (data[i+1] << 8);
        i += 2;
        // uint32_t blocks_size = data[i++] | (data[i++] << 8);
        uint8_t * b = (uint8_t *)&data[i];
        i += blocks_size;
        uint8_t cm[((map_size * map_size * map_size) >> 3) * sizeof(uint8_t)] = {0};
        // todo, globalize and whatever;
        // block_t * r_blocks = NULL;
        vector * blocks = vector_init(sizeof(block_t));
        // uint32_t r_size = 0;
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

            // todo, globalize, and never free?
            // r_size++;
            // r_blocks = realloc(r_blocks, sizeof(block_t) * r_size);
            vector_push(blocks, &(block_t) {
                .t = t,
                .b = r_push_block(x << 5, y << 4, z << 5, sx << 5, sy << 4, sz << 5, t),
            });
            // r_blocks[r_size - 1] = (block_t) {
            //     .t = t,
            //     .b = r_push_block(x << 5, y << 4, z << 5, sx << 5, sy << 4, sz << 5, t),
            // };

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

        // todo, gross
        tmp_maps[tmp_maps_len - 1] = (map_t) {
            0
        };
        memcpy(tmp_maps[tmp_maps_len - 1].cm, cm, ((map_size * map_size * map_size) >> 3) * sizeof(uint8_t));
        tmp_maps[tmp_maps_len - 1].e = e;
        tmp_maps[tmp_maps_len - 1].e_size = e_size;
        tmp_maps[tmp_maps_len - 1].blocks = blocks;
        // todo, should this be = r_size - 1??
        // tmp_maps[tmp_maps_len - 1].r_size = r_size;
    }
    return (map_collection_t) {
        .maps = tmp_maps,
        .len = tmp_maps_len
    };
}

void map_load (map_t * m) {
    // todo, should this just be an index into a global map_collection_t?
    map = m;

    // backup -- entity_constructor
    // Entity Id to class - must be consistent with map_packer.c line ~900
    // void (*spawn_class[])(entity_t *, vec3_t, uint8_t, uint8_t) = {
    //     /* 00 */ entity_player_constructor,
    //     /* 01 */ entity_enemy_grunt_constructor,
    //     /* 02 */ entity_enemy_enforcer_constructor,
    //     /* 03 */ entity_enemy_ogre_constructor,
    //     /* 04 */ entity_enemy_zombie_constructor,
    //     /* 05 */ entity_enemy_hound_constructor,
    //     /* 06 */ entity_pickup_nailgun_constructor,
    //     /* 07 */ entity_pickup_grenadelauncher_constructor,
    //     /* 08 */ entity_pickup_health_constructor,
    //     /* 09 */ entity_pickup_nails_constructor,
    //     /* 10 */ entity_pickup_grenades_constructor,
    //     /* 11 */ entity_barrel_constructor,
    //     /* 12 */ entity_light_constructor,
    //     /* 13 */ entity_trigger_level_constructor,
    //     /* 14 */ entity_door_constructor,
    //     /* 15 */ entity_pickup_key_constructor,
    //     /* 16 */ entity_torch_constructor,
    // };
    void (*spawn_class[])(entity_t *, vec3_t, uint8_t, uint8_t) = { // todo, obv
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
        p2
        );
    }
}

uint8_t map_block_at(int32_t x, int32_t y, int32_t z) {
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
        for (uint32_t y = (uint32_t)(box_start.y) >> 4; y <= (int32_t)(box_end.y) >> 4; y++) {
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

    for (uint32_t i = 0; i < vector_size(blocks); i++) {

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

void map_quit() {
    if (map_data.maps) {
        for(int i = 0; i < map_data.len; i++) {
            vector_free(map_data.maps[i].blocks);
        }
        free(map_data.maps);
    }
}