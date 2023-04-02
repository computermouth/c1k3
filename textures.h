#ifndef _TEXTURES_
#define _TEXTURES_

#include <stdint.h>

// extern const uint8_t ** textures;

typedef struct {
    uint8_t * data;
    uint64_t len;
} png_bin_t;

extern const png_bin_t * textures;

#endif
