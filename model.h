
#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>

#include "math.h"
#include "vector.h"
#include "map.h"

typedef struct {
    uint32_t * frames;
    uint32_t frame_len;
    uint32_t nv;
} model_t;

vector * model_load(void * void_verts, uint32_t frame_len, uint32_t vert_len, float * u, float * v);
void model_quit();

#endif
