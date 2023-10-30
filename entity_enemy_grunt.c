
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

void entity_enemy_grunt_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {

    char * str_p1 = entity_param_lookup("patrol", e->_params->entity_generic_params.extras);
    if (str_p1)
        p1 = atoi(str_p1);
    else
        p1 = 0;

    entity_enemy_constructor(e, pos, p1, p2);
    e->_init = entity_enemy_grunt_init;
    e->_attack = entity_enemy_grunt_attack;
    e->_init(e, p1, p2);

    // todo, move everything from here on to grunt_init
    entity_parse_animation_frames(
        e->_params->entity_generic_params.ref_entt,
        grunt_animations,
        sizeof(grunt_animations)/sizeof(grunt_animations[0]),
        last_ref_entt
    );

    e->_texture = e->_params->entity_generic_params.ref_entt->tex_id;
    vector * frames = e->_params->entity_generic_params.ref_entt->frames;
    uint32_t * uframes = vector_begin(frames);
    e->_model.frames = uframes;
    e->_model.nv = e->_params->entity_generic_params.ref_entt->vert_len;
    e->s = e->_params->entity_generic_params.ref_entt->size;
}

void entity_enemy_grunt_init(entity_t * e, uint8_t patrol_dir, uint8_t p2) {
    // e->_model = &(model_grunt);
    e->_texture = 17;
    e->_health = 40;

    e->_animation_collection = (animation_collection_t) {
        .animations = grunt_animations,
        .num_animations = sizeof(grunt_animations)/sizeof(grunt_animations[0]),
    };

    e->_set_state(e, e->_state);
}

void entity_enemy_grunt_attack(entity_t * e) {
    e->_play_sound(e, sfx_shotgun_shoot);
    entity_t * tmplight = game_spawn(entity_light_constructor, vec3_add(e->p, vec3(0,30,0)), 10, 0xff, NULL);
    tmplight->_expires = true;
    tmplight->_die_at = game_time + 0.1;

    for (int32_t i = 0; i < 3; i++) {
        e->_spawn_projectile(
            e,
            entity_projectile_shell_constructor, 10000,
            randf() * 0.08 - 0.04, randf() * 0.08 - 0.04
        );
    }
}