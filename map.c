
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "map.h"
#include "data.h"
#include "render.h"

void * map;

map_collection_t map_load_container() {

    const uint8_t * data = data_maps;
    map_t * tmp_maps = NULL;
    uint32_t tmp_maps_len = 0;
    for(uint32_t i = 0; i < data_maps_len;) {
        tmp_maps_len++;
        tmp_maps = realloc(tmp_maps, sizeof(map_t) * tmp_maps_len);

        uint32_t blocks_size = data[i++] | (data[i++] << 8);
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
        uint32_t num_entities = data[i++] | (data[i++] << 8);
        uint8_t * e = (uint8_t *)(data + i);
        uint32_t e_size = num_entities * 6;
        i += e_size;

        // todo, gross
        memcpy(tmp_maps[tmp_maps_len - 1].cm, cm, ((map_size * map_size * map_size) >> 3) * sizeof(uint8_t));
        tmp_maps[tmp_maps_len - 1].e = e;
        tmp_maps[tmp_maps_len - 1].e_size = e_size;
        tmp_maps[tmp_maps_len - 1].r = r;
    }
    return (map_collection_t) {
        .maps = tmp_maps,
        .len = tmp_maps_len
    };
}