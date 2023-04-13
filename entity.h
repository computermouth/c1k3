
#ifndef _ENTITY_
#define _ENTITY_

#include <stdint.h>
#include <stdbool.h>

#include "math.h"
#include "model.h"

typedef enum {
    ENTITY_GROUP_NONE = 0,
    ENTITY_GROUP_PLAYER = 1,
    ENTITY_GROUP_ENEMY = 2,
} entity_group_t;

typedef struct {
    float time;
    uint32_t num_frames;
    uint32_t * frames;
} animation_t;

typedef struct {
    vec3_t a;
    vec3_t v;
    vec3_t p;
    vec3_t s;
    float f; // friction?

    model_t * _model;
    animation_t _anim;
    int32_t _anim_time;
    int32_t _texture;

    int32_t _health;
    int32_t _dead;
    bool _expires; // to be used with die at
    float _die_at; // float because it's compared to game_tim
    int32_t _step_height;
    float _bounciness;
    int32_t _gravity;
    float _yaw;
    float _pitch;
    int32_t _on_ground;
    int32_t _keep_off_ledges;
    int32_t _speed;
    float _bob;
    bool _can_jump;
    float _can_shoot_at;
    uint32_t _light;
    float _spawn_time;
    bool _flicker;
    uint8_t _color[3];

    void * _weapons;
    uint32_t _weapon_index;

    int32_t _check_against;
    void * _check_entities;
    int32_t _stepped_up_at;

    // first param is actually entity_t *
    void (*_init)(void * e, uint8_t p1, uint8_t p2);
    void (*_update)(void * e);
    void (*_update_physics)(void * e);
    bool (*_collides)(void * e, vec3_t p);
    void (*_did_collide)(int axis);
    void (*_did_collide_with_entity)(void * e, void * other);
    void (*_draw_model)(void * e);
    void (*_spawn_particles)(void * e, int amount, int speed, model_t * model, int texture, float lifetime);
    void (*_receive_damage)(void * e, void * from, int32_t amount);
    void (*_play_sound)(void * e, void * sound);
    void (*_kill)(void * e);

} entity_t;

typedef struct {
    entity_t * entities;
    uint32_t length;
} entity_collection_t;

// todo, fuck, get rid of this entirely?
// loop over entire entity collection and check ENTITY_GROUP?
typedef struct {
    entity_t ** entities;
    uint32_t length;
} entity_ref_collection_t;

// todo, jesus christ, I think p1 and p2 are typeless and subject to usage
// might have to pass void pointers and hope for the best
void entity_constructor(entity_t *e, vec3_t pos, uint8_t p1, uint8_t p2);
void entity_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_update(entity_t * e);
void entity_update_physics(entity_t * e);
bool entity_collides(entity_t * e, vec3_t p);
void entity_did_collide(int axis); // todo, axis should probably be an enum
void entity_did_collide_with_entity(entity_t * e, entity_t * other);
void entity_draw_model(entity_t * e);
void entity_spawn_particles(entity_t * e, int amount, int speed, model_t * model, int texture, float lifetime);
void entity_receive_damage(entity_t * e, entity_t * from, int32_t amount);
void entity_play_sound(entity_t * e, void * sound);
void entity_kill(entity_t * e);

#endif
