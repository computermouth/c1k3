#ifndef _DATA_
#define _DATA_

#include <stdint.h>

typedef struct {
    uint8_t * data;
    uint64_t len;
} png_bin_t;

extern const png_bin_t * data_textures;
extern const uint32_t data_textures_len;

extern const uint8_t * data_maps;
extern const uint32_t data_maps_len;

extern const uint8_t * data_models;
extern const uint32_t data_models_len;

#endif
