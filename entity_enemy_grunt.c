
#include "entity.h"
#include "entity_enemy.h"
#include "entity_projectile_shell.h"
#include "entity_light.h"
#include "game.h"
#include "audio.h"
#include "map.h"
#include "vector.h"

void entity_enemy_grunt_init(entity_t * e);
void entity_enemy_grunt_attack(entity_t * e);

animation_t grunt_animations[] = {
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

void entity_enemy_grunt_constructor(entity_t * e, vec3_t pos) {

    char * str_p1 = entity_param_lookup("patrol", e->_params->entity_generic_params.extras);
    uint8_t patrol = 0;
    if (str_p1)
        patrol = atoi(str_p1);

    entity_enemy_constructor(e, pos, patrol);
    e->_attack = entity_enemy_grunt_attack;
    entity_enemy_grunt_init(e);
}

void entity_enemy_grunt_init(entity_t * e) {
    e->_health = 40;

    entity_parse_animation_frames(
        e->_params->entity_generic_params.ref_entt,
        grunt_animations,
        sizeof(grunt_animations)/sizeof(grunt_animations[0]),
        &last_ref_entt
    );

    entity_set_model(e);

    e->_animation_collection = (animation_collection_t) {
        .animations = grunt_animations,
        .num_animations = sizeof(grunt_animations)/sizeof(grunt_animations[0]),
    };

    e->_set_state(e, e->_state);
}

void entity_enemy_grunt_attack(entity_t * e) {
    e->_play_sound(e, sfx_shotgun_shoot);


    entity_params_t l = {
        .id = ENTITY_ID_LIGHT,
        .entity_light_params = {
            .position = vec3_add(e->p, vec3(0,30,0)),
            .rgba[0] = 0xff,
            .rgba[1] = 0xff,
            .rgba[2] = 0xff,
            .rgba[3] = 0x0a,
        },
    };
    entity_t * tmplight = game_spawn_ng(&l);

    tmplight->_expires = true;
    tmplight->_die_at = game_time + 0.1;

    for (int32_t i = 0; i < 3; i++) {
        e->_spawn_projectile(
            e,
            ENTITY_ID_PROJECTILE_SHELL, 10000,
            randf() * 0.08 - 0.04, randf() * 0.08 - 0.04
        );
    }
}