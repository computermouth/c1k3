
#ifndef _MODEL_
#define _MODEL_

#include <stdint.h>

#include "math.h"

typedef struct {
    uint8_t * data;
    uint32_t data_len;
    uint32_t * frames;
    uint32_t frame_len;
    uint32_t nv;
} model_t;

typedef struct {
    model_t * models;
    uint32_t len;
} model_collection_t;

model_collection_t model_load_container();
model_t model_init(uint8_t * data, vec3_t scale);

#endif