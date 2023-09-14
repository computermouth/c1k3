
#ifndef __MAPC_H__
#define __MAPC_H__

#include <stdint.h>
#include "vector.h"

typedef struct {
    uint64_t x;
    uint64_t y;
    uint64_t z;
} mapc_pos3_t;

typedef struct {
    float x;
    float y;
    float z;
} mapc_fpos3_t;

typedef struct {
    size_t len;
    uint8_t * data;
} mapc_txtr_t;

typedef struct {
    vector * u;
    vector * v;
    vector * anim_names;
    vector * anim_frames;
} mapc_verts_t;

typedef struct {
    mapc_txtr_t txtr;
    mapc_fpos3_t start;
    mapc_fpos3_t size;
} mapc_rm_cube_t;

typedef struct {
    mapc_verts_t verts;
    mapc_txtr_t txtr;
    mapc_fpos3_t fpos;
} mapc_rm_entt_t;

typedef struct {
    uint8_t texture;
    mapc_pos3_t start;
    mapc_pos3_t size;
} mapc_out_cube_t;

typedef struct {
    uint8_t texture;
    mapc_fpos3_t fpos;
} mapc_out_entt_t;

typedef struct {
    uint8_t color[4];
    mapc_fpos3_t fpos;
} mapc_out_lite_t;

typedef struct {
    mapc_fpos3_t fpos;
} mapc_out_plyr_t;

#endif