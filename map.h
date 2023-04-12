
#ifndef _MAP_
#define _MAP_

#include <stdint.h>

#include "math.h"

#define map_size 128

typedef struct {
    uint32_t t;
    int32_t b;
} block_t;

typedef struct {
    block_t * r;
    uint32_t r_size;
    uint8_t * e;
    uint32_t e_size;
    uint8_t cm[((map_size * map_size * map_size) >> 3)];
} map_t;

typedef struct {
    map_t * maps;
    uint32_t len;
} map_collection_t;

extern map_collection_t map_data;

map_collection_t map_load_container();
void map_init (map_t * m);
uint32_t map_block_at(uint32_t x, uint32_t y, uint32_t z);
int map_block_at_box(vec3_t box_start, vec3_t box_end);
void map_draw();

#endif