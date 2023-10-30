
#include "entity.h"
#include "entity_enemy.h"
#include "entity_projectile_shell.h"
#include "entity_light.h"
#include "game.h"
#include "audio.h"
#include "map.h"
#include "vector.h"

void entity_enemy_mutant_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_enemy_mutant_attack(entity_t * e);

// todo, once animations are actually parse-able
animation_t mutant_animations[] = {
    {   // 0: Idle
        .time = 0.25,
        .num_frames = 9,
        .frames_ng = (animation_frame_t[]) {
            {.name = "default"},
            {.name = "MutantMesh.001"},
            {.name = "MutantMesh.002"},
            {.name = "MutantMesh.003"},
            {.name = "MutantMesh.004"},
            {.name = "MutantMesh.005"},
            {.name = "MutantMesh.006"},
            {.name = "MutantMesh.007"},
            {.name = "MutantMesh.008"},
        },
    }
};

// hack for caching parsed frame names per-map
static ref_entt_t * last_ref_entt = NULL;

enemy_state_t mutant_enemy_states[_ENEMY_STATE_NULL] = {
    [ENEMY_STATE_IDLE]           = {ENEMY_ANIMATION_IDLE,   0.0, 0.8, _ENEMY_STATE_NULL},
    [ENEMY_STATE_PATROL]         = {ENEMY_ANIMATION_IDLE,   0.5, 0.8, _ENEMY_STATE_NULL},
    [ENEMY_STATE_FOLLOW]         = {ENEMY_ANIMATION_IDLE,   0.0, 0.8, _ENEMY_STATE_NULL},
    [ENEMY_STATE_ATTACK_RECOVER] = {ENEMY_ANIMATION_IDLE,   0.0, 0.8, _ENEMY_STATE_NULL},
    [ENEMY_STATE_ATTACK_EXEC]    = {ENEMY_ANIMATION_IDLE,   0.0, 0.8, _ENEMY_STATE_NULL},
    [ENEMY_STATE_ATTACK_PREPARE] = {ENEMY_ANIMATION_IDLE,   0.0, 0.8, _ENEMY_STATE_NULL},
    [ENEMY_STATE_ATTACK_AIM]     = {ENEMY_ANIMATION_IDLE,   0.0, 0.8, _ENEMY_STATE_NULL},
    [ENEMY_STATE_EVADE]          = {ENEMY_ANIMATION_IDLE,   0.0, 0.8, _ENEMY_STATE_NULL},
};

// todo, do something less stupid with this
model_t model_mutant = { 0 };

void entity_enemy_mutant_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_enemy_constructor(e, pos, p1, p2);
    e->_attack = entity_enemy_mutant_attack;
    entity_enemy_mutant_init(e, p1, p2);
}

void entity_enemy_mutant_init(entity_t * e, uint8_t patrol_dir, uint8_t p2) {
    e->_health = 40;
    e->_speed = 0;
    
    e->_STATE_IDLE              = ENEMY_STATE_IDLE;
    e->_STATE_PATROL            = ENEMY_STATE_IDLE;
    e->_STATE_FOLLOW            = ENEMY_STATE_IDLE;
    e->_STATE_ATTACK_RECOVER    = ENEMY_STATE_IDLE;
    e->_STATE_ATTACK_EXEC       = ENEMY_STATE_IDLE;
    e->_STATE_ATTACK_PREPARE    = ENEMY_STATE_IDLE;
    e->_STATE_ATTACK_AIM        = ENEMY_STATE_IDLE;
    e->_STATE_EVADE             = ENEMY_STATE_IDLE;
    e->_STATE_NULL              = _ENEMY_STATE_NULL;

    e->_set_state(e, e->_STATE_IDLE);

    entity_parse_animation_frames(
        e->_params->entity_generic_params.ref_entt,
        mutant_animations,
        sizeof(mutant_animations)/sizeof(mutant_animations[0]),
        last_ref_entt
    );

    entity_set_model(e);

    e->_state_collection = (enemy_state_collection_t) {
        .num_states = _ENEMY_STATE_NULL,
        .states = mutant_enemy_states
    };

    e->_animation_collection = (animation_collection_t) {
        .animations = mutant_animations,
        .num_animations = sizeof(mutant_animations)/sizeof(mutant_animations[0]),
    };
}

void entity_enemy_mutant_attack(entity_t * e) {}
