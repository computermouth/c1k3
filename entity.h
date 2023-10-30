
#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "math.h"
#include "model.h"
#include "weapon.h"
#include "vector.h"
#include "map.h"

typedef enum {
    ENEMY_STATE_IDLE,
    ENEMY_STATE_PATROL,
    ENEMY_STATE_FOLLOW,
    ENEMY_STATE_ATTACK_RECOVER,
    ENEMY_STATE_ATTACK_EXEC,
    ENEMY_STATE_ATTACK_PREPARE,
    ENEMY_STATE_ATTACK_AIM,
    ENEMY_STATE_EVADE,
    _ENEMY_STATE_NULL,
} enemy_state_enum_t;

typedef enum {
    ENEMY_ANIMATION_IDLE,
    ENEMY_ANIMATION_WALK,
    ENEMY_ANIMATION_RUN,
    ENEMY_ANIMATION_ATTACK_PREPARE,
    ENEMY_ANIMATION_ATTACK,
    _ENEMY_ANIMATION_NULL,
} enemy_animation_enum_t;

typedef struct {
    uint32_t index;
    float speed;
    float next_state_update;
    enemy_state_enum_t next_state;
} enemy_state_t;

typedef struct {
    uint32_t num_states;
    enemy_state_t * states;
} enemy_state_collection_t;

typedef enum {
    ENTITY_GROUP_NONE = 0,
    ENTITY_GROUP_PLAYER = 1,
    ENTITY_GROUP_ENEMY = 2,
    ENTITY_GROUP_ALL = 3,
} entity_group_t;

typedef struct {
    size_t id;
    char * name;
} animation_frame_t;

typedef struct {
    float time;
    uint32_t num_frames;
    animation_frame_t * frames_ng;
    uint32_t * frames;
    // todo, remove ^
} animation_t;

typedef struct {
    uint32_t num_animations;
    animation_t * animations;
} animation_collection_t;

typedef struct entity_t {
    vec3_t a;
    vec3_t v;
    vec3_t p;
    vec3_t s;
    float f; // friction?

    model_t _model;
    animation_collection_t _animation_collection;
    animation_t * _anim;
    float _anim_time;
    int32_t _texture;

    enemy_state_collection_t _state_collection;
    enemy_state_enum_t _state;

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
    float _light;
    float _spawn_time;
    bool _flicker;
    uint8_t _color[3];
    vec3_t _start_pos;
    float _reset_state_at;
    float _open;
    bool _needs_key;
    int32_t _damage;
    vec3_t _light_pos;
    float _target_yaw;
    float _state_update_at;
    uint32_t _attack_distance;
    uint32_t _evade_distance;
    float _attack_chance;
    float _turn_bias;
    bool _did_hit;
    int32_t _trigger_range;
    entity_params_t * _params;

    enemy_state_enum_t _STATE_IDLE;
    enemy_state_enum_t _STATE_PATROL;
    enemy_state_enum_t _STATE_FOLLOW;
    enemy_state_enum_t _STATE_ATTACK_RECOVER;
    enemy_state_enum_t _STATE_ATTACK_EXEC;
    enemy_state_enum_t _STATE_ATTACK_PREPARE;
    enemy_state_enum_t _STATE_ATTACK_AIM;
    enemy_state_enum_t _STATE_EVADE;
    enemy_state_enum_t _STATE_NULL;

    weapon_t _weapons[WEAPON_END];
    weapon_type_t _weapon_index;

    int32_t _group;
    int32_t _check_against;
    vector * _check_entities;
    float _stepped_up_at;

    // first param is actually entity_t *
    void (*_init)(struct entity_t * e, uint8_t p1, uint8_t p2);
    void (*_update)(struct entity_t * e);
    void (*_update_physics)(struct entity_t * e);
    bool (*_collides)(struct entity_t * e, vec3_t p);
    void (*_did_collide)(struct entity_t * e, int axis);
    void (*_did_collide_with_entity)(struct entity_t * e, struct entity_t * other);
    void (*_draw_model)(struct entity_t * e);
    void (*_spawn_particles)(struct entity_t * e, int amount, int speed, model_t model, int texture, float lifetime);
    void (*_spawn_particles_ng)(struct entity_t * e, int amount, int speed, entity_id_t eid, float lifetime);
    void (*_set_state)(struct entity_t * e, uint32_t state);
    struct entity_t * (*_spawn_projectile)(struct entity_t * e, void (*func)(struct entity_t *, vec3_t, uint8_t, uint8_t), float speed, float yaw_offset, float pitch_offset);
    struct entity_t * (*_spawn_projectile_ng)(struct entity_t * e, entity_id_t eid, float speed, float yaw_offset, float pitch_offset);
    void (*_receive_damage)(struct entity_t * e, struct entity_t * from, int32_t amount);
    void (*_play_sound)(struct entity_t * e, Mix_Chunk * sound);
    void (*_kill)(struct entity_t * e);
    void (*_pickup)(struct entity_t * e);
    void (*_attack)(struct entity_t * e);
} entity_t;

int64_t entity_frame_from_name(char * needle, char (*haystack)[][100], size_t len);
char * entity_param_lookup(char * key, vector * v);
void entity_parse_animation_frames(ref_entt_t * curr_entt, animation_t * animations, size_t anim_len, ref_entt_t * last_entt);
void entity_set_model(entity_t * e);
void entity_constructor(entity_t *e, vec3_t pos, uint8_t p1, uint8_t p2);
void entity_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_update(entity_t * e);
void entity_update_physics(entity_t * e);
bool entity_collides(entity_t * e, vec3_t p);
void entity_did_collide(entity_t * e, int axis); // todo, axis should probably be an enum
void entity_did_collide_with_entity(entity_t * e, entity_t * other);
void entity_draw_model(entity_t * e);
void entity_spawn_particles(entity_t * e, int amount, int speed, model_t model, int texture, float lifetime);
void entity_spawn_particles_ng(entity_t * e, int amount, int speed, entity_id_t eid, float lifetime);
void entity_receive_damage(entity_t * e, entity_t * from, int32_t amount);
void entity_play_sound(entity_t * e, Mix_Chunk * sound);
void entity_kill(entity_t * e);
void entity_pickup(entity_t * e);
void entity_set_state(entity_t * e, uint32_t state);
void entity_attack(entity_t * e);
entity_t * entity_spawn_projectile(entity_t * e, void (*func)(entity_t *, vec3_t, uint8_t, uint8_t), float speed, float yaw_offset, float pitch_offset);
entity_t * entity_enemy_spawn_projectile_ng(entity_t * e, entity_id_t eid, float speed, float yaw_offset, float pitch_offset);

#endif
