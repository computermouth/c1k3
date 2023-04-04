
#ifndef _MAP_
#define _MAP_

#include <stdint.h>

#define map_size 128

typedef struct {
    uint32_t t;
    int32_t b;
} block_t;

typedef struct {
    block_t * r;
    uint8_t * e;
    uint32_t e_size;
    uint8_t cm[((map_size * map_size * map_size) >> 3)];
} map_t;

typedef struct {
    map_t * maps;
    uint32_t len;
} map_collection_t;

map_collection_t map_load_container();

#endif