
#ifndef _WEAPON_
#define _WEAPON_

#include <stdint.h>
#include <stdbool.h>

#include "math.h"
#include "model.h"

typedef enum {
    WEAPON_SHOTGUN,
    WEAPON_NAILGUN,
    WEAPON_GRENADELAUNCHER,
    WEAPON_END
} weapon_type_t;

typedef struct {
    bool _needs_ammo;
    vec3_t _projectile_offset;
    uint32_t _ammo;
    void * _sound;
    // todo, rename to projectile_constructor or something
    void (*_projectile_type)(void *, vec3_t, uint8_t, uint8_t);
    float _projectile_speed;
    uint32_t _texture;
    model_t * _model;
    float _reload;
    void (*_init)(void * w);
    void (*_shoot)(void * w, vec3_t pos, float yaw, float pitch);
    void (*_spawn_projectile)(void * w, vec3_t pos, float yaw, float pitch);
} weapon_t;

weapon_t weapon_shotgun_constructor();
weapon_t weapon_nailgun_constructor();
weapon_t weapon_grenadelauncher_constructor();

#endif