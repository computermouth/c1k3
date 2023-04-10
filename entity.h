
#ifndef _ENTITY_
#define _ENTITY_

#include <stdint.h>

#include "math.h"

typedef enum {
    ENTITY_GROUP_NONE = 0,
    ENTITY_GROUP_PLAYER = 1,
    ENTITY_GROUP_ENEMY = 2,
} entity_group_t;

typedef struct {
    uint32_t length;
    uint32_t * no_idea;
} animation_t;

typedef struct {
    vec3_t a;
    vec3_t v;
    vec3_t p;
    vec3_t s;
    float f;

    void * _model;
    animation_t _anim;
    int32_t _anim_time;

    int32_t _health;
    int32_t _dead;
    int32_t _die_at;
    int32_t _step_height;
    int32_t _bounciness;
    int32_t _gravity;
    float _yaw;
    float _pitch;
    int32_t _on_ground;
    int32_t _keep_off_ledges;

    int32_t _check_against;
    int32_t _stepped_up_at;

} entity_t;

void entity_constructor(entity_t * e, vec3_t pos, vec3_t p1, vec3_t p2);
void entity_init(entity_t * e, vec3_t p1, vec3_t p2);
void entity_update(entity_t * e);
void entity_update_physics(entity_t * e);
void entity_collides(entity_t * e);
void entity_did_collide(entity_t * e);

#endif