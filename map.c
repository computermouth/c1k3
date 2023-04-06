
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "map.h"
#include "data.h"
#include "math.h"
#include "render.h"

map_t * map;

map_collection_t map_load_container() {

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
        uint8_t cm[((map_size * map_size * map_size) >> 3) * sizeof(uint8_t)];
        // todo, globalize and whatever;
        block_t * r = NULL;
        uint32_t r_size = 0;
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
            r_size++;
            r = realloc(r, sizeof(block_t) * r_size);
            r[r_size - 1] = (block_t) {
                .t = t,
                .b = r_push_block(x << 5, y << 4, z << 5, sx << 5, sy << 4, sz << 5, t),
            };

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
        memcpy(tmp_maps[tmp_maps_len - 1].cm, cm, ((map_size * map_size * map_size) >> 3) * sizeof(uint8_t));
        tmp_maps[tmp_maps_len - 1].e = e;
        tmp_maps[tmp_maps_len - 1].e_size = e_size;
        tmp_maps[tmp_maps_len - 1].r = r;
        // todo, should this be = r_size - 1??
        tmp_maps[tmp_maps_len - 1].r_size = r_size;
    }
    return (map_collection_t) {
        .maps = tmp_maps,
        .len = tmp_maps_len
    };
}

void map_init (map_t * m) {
    // todo, should this just be an index into a global map_collection_t?
    map = m;

    printf("map.c/map_init() -- todo");

    // entity_t spawn_class[] = {

    // }

}

uint32_t map_block_at(uint32_t x, uint32_t y, uint32_t z) {
    return map->cm[
               (
                   z * map_size * map_size +
                   y * map_size +
                   x
               ) >> 3
        ] & (1 << (x & 7));
}

vec3_t * map_trace(vec3_t * a, vec3_t * b) {
    vec3_t diff = vec3_sub(*a, *b);
    vec3_t step_dir = vec3_mulf(vec3_normalize(diff), 16);
    float steps = vec3_length(diff) / 16;

    for (uint32_t i = 0; i < steps; i++) {
        *a = vec3_add(*a, step_dir);
        if (map_block_at((int32_t)(a->x) >> 5, (int32_t)(a->y) >> 4, (int32_t)(a->z) >> 5)) {
            return a;
        }
    }

    return NULL;
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
    for (uint32_t i = 0; i < map->r_size; i++) {

        draw_call_t call = {
            .pos = {0.0f, 0.0f, 0.0f},
            .yaw = 0.0f,
            .pitch = 0.0f,
            .texture = map->r[i].t,
            .f1 = map->r[i].b,
            .f2 = map->r[i].b,
            .mix = 0.0f,
            .num_verts = 36
        };

        r_draw(call);
    }
}