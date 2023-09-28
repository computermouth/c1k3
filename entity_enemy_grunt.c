
#include "entity.h"
#include "entity_enemy.h"
#include "entity_projectile_shell.h"
#include "entity_light.h"
#include "game.h"
#include "audio.h"
#include "map.h"
#include "vector.h"

void entity_enemy_grunt_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_enemy_grunt_attack(entity_t * e);

// todo, once animations are actually parse-able
animation_t grunt_animations[] = {
    {   // 0: Idle
        .time = 1,
        .num_frames = 1,
        // .frames = (uint32_t[]){0, 0},
        .frames_ng = (animation_frame_t[]) {
            {.name = "default"},
        },
    },
    {   // 1: Walk
        .time = 0.40f,
        .num_frames = 4,
        .frames_ng = (animation_frame_t[]) {
            {.name = "run_1"},
            {.name = "run_2"},
            {.name = "run_3"},
            {.name = "run_4"},
        },
    },
    {   // 2: Run
        .time = 0.20f,
        .num_frames = 4,
        .frames_ng = (animation_frame_t[]) {
            {.name = "run_1"},
            {.name = "run_2"},
            {.name = "run_3"},
            {.name = "run_4"},
        },
    },
    {   // 3: Attack prepare
        .time = 0.25f,
        .num_frames = 4,
        .frames_ng = (animation_frame_t[]) {
            {.name = "default"},
            {.name = "shoot"},
            {.name = "shoot"},
            {.name = "shoot"},
        },
    },
    {   // 4: Attack
        .time = 0.25f,
        .num_frames = 4,
        .frames_ng = (animation_frame_t[]) {
            {.name = "shoot"},
            {.name = "default"},
            {.name = "default"},
            {.name = "default"},
        },
    },
};

// hack for caching parsed frame names per-map
static ref_entt_t * last_ref_entt = NULL;

enemy_state_t grunt_enemy_states[_ENEMY_STATE_NULL] = {
    {ENEMY_ANIMATION_IDLE,   0, 0.1, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_WALK, 0.5, 0.5, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_RUN,   1, 0.3, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_IDLE,   0, 0.1, ENEMY_STATE_FOLLOW},
    {ENEMY_ANIMATION_ATTACK,   0, 0.4, ENEMY_STATE_ATTACK_RECOVER},
    {ENEMY_ANIMATION_ATTACK_PREPARE,   0, 0.4, ENEMY_STATE_ATTACK_EXEC},
    {ENEMY_ANIMATION_IDLE,   0, 0.1, ENEMY_STATE_ATTACK_PREPARE},
    {ENEMY_ANIMATION_RUN,   1, 0.8, ENEMY_STATE_ATTACK_AIM},
};

void entity_enemy_grunt_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {

    char * str_p1 = entity_param_lookup("patrol", e->_params->entity_generic_params.extras);
    if (str_p1)
        p1 = atoi(str_p1);
    else
        p1 = 0;

    entity_enemy_constructor(e, pos, p1, p2);
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_enemy_grunt_init;
    e->_attack = (void (*)(void *))entity_enemy_grunt_attack;
    e->_init(e, p1, p2);

    // todo, move everything from here on to grunt_init
    entity_parse_animation_frames(
        e->_params->entity_generic_params.ref_entt,
        grunt_animations,
        sizeof(grunt_animations)/sizeof(grunt_animations[0]),
        last_ref_entt
    );

    e->_texture = e->_params->entity_generic_params.ref_entt->tex_id;
    // hack for old models
    free(e->_model->frames);
    e->_model->frames = vector_begin(e->_params->entity_generic_params.ref_entt->frames);

    e->_state_collection = (enemy_state_collection_t) {
        .num_states = _ENEMY_STATE_NULL,
        .states = grunt_enemy_states
    };

    e->_animation_collection = (animation_collection_t) {
        .animations = grunt_animations,
        .num_animations = sizeof(grunt_animations)/sizeof(grunt_animations[0]),
    };

    e->_set_state(e, e->_state);
}

void entity_enemy_grunt_init(entity_t * e, uint8_t patrol_dir, uint8_t p2) {
    e->_model = &(model_grunt);
    e->_texture = 17;
    e->_health = 40;
}

void entity_enemy_grunt_attack(entity_t * e) {
    e->_play_sound(e, sfx_shotgun_shoot);
    entity_t * tmplight = game_spawn(entity_light_constructor, vec3_add(e->p, vec3(0,30,0)), 10, 0xff, NULL);
    tmplight->_expires = true;
    tmplight->_die_at = game_time + 0.1;

    for (int32_t i = 0; i < 3; i++) {
        e->_spawn_projectile(
            e,
            (void (*)(void *, vec3_t, uint8_t, uint8_t))entity_projectile_shell_constructor, 10000,
            randf() * 0.08 - 0.04, randf() * 0.08 - 0.04
        );
    }
}