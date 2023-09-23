
#include "entity.h"
#include "entity_enemy.h"
#include "entity_projectile_shell.h"
#include "entity_light.h"
#include "game.h"
#include "audio.h"
#include "map.h"
#include "vector.h"

void entity_enemy_mutant_init(entity_t * e, uint8_t p1, uint8_t p2);

// todo, once animations are actually parse-able
animation_t mutant_animations[] = {
    {   // 0: Idle
        .time = 0.25,
        .num_frames = 14,
        .frames_ng = (animation_frame_t[]) {
            {.name = "default"},
            {.name = "d_01"},
            {.name = "d_02"},
            {.name = "d_03"},
            {.name = "d_04"},
            {.name = "d_05"},
            {.name = "d_06"},
            {.name = "d_07"},
            {.name = "d_08"},
            {.name = "d_09"},
            {.name = "d_10"},
            {.name = "d_11"},
            {.name = "d_12"},
            {.name = "d_13"},
        },
    }
};

// hack for caching parsed frame names per-map
static ref_entt_t * last_ref_entt = NULL;

enemy_state_t mutant_enemy_states[_ENEMY_STATE_NULL] = {
    {ENEMY_ANIMATION_IDLE,   1, 0.8, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_IDLE,   1, 0.8, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_IDLE,   1, 0.8, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_IDLE,   1, 0.8, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_IDLE,   1, 0.8, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_IDLE,   1, 0.8, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_IDLE,   1, 0.8, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_IDLE,   1, 0.8, _ENEMY_STATE_NULL},
};

// todo, do something less stupid with this
model_t model_mutant = { 0 };

void entity_enemy_mutant_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2, entity_params_t * ep) {

    entity_enemy_constructor(e, pos, p1, p2, ep);
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_enemy_mutant_init;
    e->_init(e, p1, p2);

    // todo, move everything from here on to mutant_init
    entity_parse_animation_frames(
        ep->entity_generic_params.ref_entt,
        mutant_animations,
        sizeof(mutant_animations)/sizeof(mutant_animations[0]),
        last_ref_entt
    );
    e->_STATE_IDLE              = ENEMY_STATE_IDLE;
    e->_STATE_PATROL            = ENEMY_STATE_IDLE;
    e->_STATE_FOLLOW            = ENEMY_STATE_IDLE;
    e->_STATE_ATTACK_RECOVER    = ENEMY_STATE_IDLE;
    e->_STATE_ATTACK_EXEC       = ENEMY_STATE_IDLE;
    e->_STATE_ATTACK_PREPARE    = ENEMY_STATE_IDLE;
    e->_STATE_ATTACK_AIM        = ENEMY_STATE_IDLE;
    e->_STATE_EVADE             = ENEMY_STATE_IDLE;
    e->_STATE_NULL              = _ENEMY_STATE_NULL;

    e->_texture = ep->entity_generic_params.ref_entt->tex_id;
    entity_generic_params_t egp = ep->entity_generic_params;
    ref_entt_t * re = egp.ref_entt;
    vector * frames = re->frames;
    uint32_t * uframes = vector_begin(frames);
    e->_model->frames = uframes;
    e->_model->nv = re->vert_len;

    e->_state_collection = (enemy_state_collection_t) {
        .num_states = _ENEMY_STATE_NULL,
        .states = mutant_enemy_states
    };

    e->_animation_collection = (animation_collection_t) {
        .animations = mutant_animations,
        .num_animations = sizeof(mutant_animations)/sizeof(mutant_animations[0]),
    };

    e->_set_state(e, e->_STATE_IDLE);
}

void entity_enemy_mutant_init(entity_t * e, uint8_t patrol_dir, uint8_t p2) {
    e->_health = 40;
    e->_speed = 0;
    e->_model = &model_mutant;
}
