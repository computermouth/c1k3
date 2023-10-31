
#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>

#include "math.h"
#include "vector.h"
#include "map.h"

typedef struct {
    uint8_t * data;
    uint32_t data_len;
    uint32_t * frames;
    uint32_t frame_len;
    uint32_t nv;
} model_t;

typedef struct {
    vector * frames;
    uint32_t nv;
} model_ng_t;

vector * model_load_ng(void * void_verts, uint32_t frame_len, uint32_t vert_len, float * u, float * v);
void model_quit();

#endif
