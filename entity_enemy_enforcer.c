
#include "entity.h"
#include "entity_enemy.h"
#include "entity_projectile_plasma.h"

#include "audio.h"
#include "map.h"

void entity_enemy_enforcer_init(entity_t * e);
void entity_enemy_enforcer_attack(entity_t * e);

animation_t enforcer_animations[] = {
    {   // 0: Idle
        .time = 1,
        .num_frames = 1,
        .frames = (animation_frame_t[]) {
            {.name = "default"},
        },
    },
    {   // 1: Walk
        .time = 0.40f,
        .num_frames = 4,
        .frames = (animation_frame_t[]) {
            {.name = "run_1"},
            {.name = "run_2"},
            {.name = "run_3"},
            {.name = "run_4"},
        },
    },
    {   // 2: Run
        .time = 0.20f,
        .num_frames = 4,
        .frames = (animation_frame_t[]) {
            {.name = "run_1"},
            {.name = "run_2"},
            {.name = "run_3"},
            {.name = "run_4"},
        },
    },
    {   // 3: Attack prepare
        .time = 0.25f,
        .num_frames = 4,
        .frames = (animation_frame_t[]) {
            {.name = "default"},
            {.name = "shoot"},
            {.name = "shoot"},
            {.name = "shoot"},
        },
    },
    {   // 4: Attack
        .time = 0.25f,
        .num_frames = 4,
        .frames = (animation_frame_t[]) {
            {.name = "shoot"},
            {.name = "default"},
            {.name = "default"},
            {.name = "default"},
        },
    },
};

// hack for caching parsed frame names per-map
static ref_entt_t * last_ref_entt = NULL;

vec3_t entity_get_size(model_t * model) {
    vec3_t size = { 0 };

    for(size_t i = 0; i < model->nv; i++) {}

    return size;
}

void entity_enemy_enforcer_constructor(entity_t *e) {

    char * str_p1 = entity_param_lookup("patrol", e->_params->entity_generic_params.extras);
    uint8_t patrol = 0;
    if (str_p1)
        patrol = atoi(str_p1);

    entity_enemy_constructor(e, patrol);
    e->_attack = entity_enemy_enforcer_attack;
    entity_enemy_enforcer_init(e);
}

void entity_enemy_enforcer_init(entity_t * e) {
    e->_health = 80;
    e->s = vec3(14,44,14);

    entity_parse_animation_frames(
        e->_params->entity_generic_params.ref_entt,
        enforcer_animations,
        sizeof(enforcer_animations)/sizeof(enforcer_animations[0]),
        &last_ref_entt
    );

    e->_animation_collection = (animation_collection_t) {
        .animations = enforcer_animations,
        .num_animations = sizeof(enforcer_animations)/sizeof(enforcer_animations[0]),
    };

    entity_set_model(e);
}

void entity_enemy_enforcer_attack(entity_t * e) {
    e->_play_sound(e, sfx_plasma_shoot);
    e->_spawn_projectile(e, ENTITY_ID_PROJECTILE_PLASMA, 800, 0, 0);
}