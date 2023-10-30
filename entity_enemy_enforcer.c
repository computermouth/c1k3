
#include "entity.h"
#include "entity_enemy.h"
#include "entity_projectile_plasma.h"

#include "audio.h"
#include "map.h"

void entity_enemy_enforcer_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_enemy_enforcer_attack(entity_t * e);

animation_t enforcer_animations[] = {
    {   // 0: Idle
        .time = 1,
        .num_frames = 1,
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

vec3_t entity_get_size(model_t * model) {
    vec3_t size = { 0 };

    for(size_t i = 0; i < model->nv; i++) {}

    return size;
}

void entity_enemy_enforcer_constructor(entity_t *e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_enemy_constructor(e, pos, p1, p2);
    e->_attack = entity_enemy_enforcer_attack;
    entity_enemy_enforcer_init(e, p1, p2);
}

void entity_enemy_enforcer_init(entity_t * e, uint8_t patrol_dir, uint8_t p2) {
    e->_health = 80;
    e->s = vec3(14,44,14);

    entity_parse_animation_frames(
        e->_params->entity_generic_params.ref_entt,
        enforcer_animations,
        sizeof(enforcer_animations)/sizeof(enforcer_animations[0]),
        last_ref_entt
    );

    e->_animation_collection = (animation_collection_t) {
        .animations = enforcer_animations,
        .num_animations = sizeof(enforcer_animations)/sizeof(enforcer_animations[0]),
    };
    
    entity_set_model(e);
}

void entity_enemy_enforcer_attack(entity_t * e) {
    e->_play_sound(e, sfx_plasma_shoot);
    e->_spawn_projectile_ng(e, ENTITY_ID_PROJECTILE_PLASMA, 800, 0, 0);
}