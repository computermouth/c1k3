
#include "entity.h"
#include "entity_enemy.h"
#include "entity_projectile_gib.h"
#include "audio.h"

animation_t zombie_animations[] = {
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
            {.name = "default"},
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

enemy_state_t zombie_enemy_states[_ENEMY_STATE_NULL] = {
    [ENEMY_STATE_IDLE]           = {ENEMY_ANIMATION_IDLE,           0.0, 0.1, _ENEMY_STATE_NULL},
    [ENEMY_STATE_PATROL]         = {ENEMY_ANIMATION_WALK,           0.5, 0.5, _ENEMY_STATE_NULL},
    [ENEMY_STATE_FOLLOW]         = {ENEMY_ANIMATION_IDLE,           0.0, 0.1, _ENEMY_STATE_NULL},
    [ENEMY_STATE_ATTACK_RECOVER] = {ENEMY_ANIMATION_IDLE,           0.0, 1.1, ENEMY_STATE_IDLE},
    [ENEMY_STATE_ATTACK_EXEC]    = {ENEMY_ANIMATION_ATTACK,         0.0, 0.4, ENEMY_STATE_ATTACK_RECOVER},
    [ENEMY_STATE_ATTACK_PREPARE] = {ENEMY_ANIMATION_ATTACK_PREPARE, 0.0, 0.4, ENEMY_STATE_ATTACK_EXEC},
    [ENEMY_STATE_ATTACK_AIM]     = {ENEMY_ANIMATION_IDLE,           0.0, 0.1, ENEMY_STATE_ATTACK_PREPARE},
    [ENEMY_STATE_EVADE]          = {ENEMY_ANIMATION_IDLE,           0.0, 0.1, ENEMY_STATE_ATTACK_AIM},
};

void entity_enemy_zombie_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_enemy_zombie_receive_damage(entity_t * e, entity_t *from, int32_t amount);
void entity_enemy_zombie_attack(entity_t * e);

void entity_enemy_zombie_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_enemy_constructor(e, pos, p1, p2);
    e->_init = entity_enemy_zombie_init;
    e->_receive_damage = entity_enemy_zombie_receive_damage;
    e->_attack = entity_enemy_zombie_attack;
    e->_init(e, p1, p2);

    // todo, move everything from here on to grunt_init
    entity_parse_animation_frames(
        e->_params->entity_generic_params.ref_entt,
        zombie_animations,
        sizeof(zombie_animations)/sizeof(zombie_animations[0]),
        last_ref_entt
    );

    e->_texture = e->_params->entity_generic_params.ref_entt->tex_id;
    vector * frames = e->_params->entity_generic_params.ref_entt->frames;
    uint32_t * uframes = vector_begin(frames);
    e->_model->frames = uframes;
    e->_model->nv = e->_params->entity_generic_params.ref_entt->vert_len;
    e->s = e->_params->entity_generic_params.ref_entt->size;
}

void entity_enemy_zombie_init(entity_t * e, uint8_t patrol_dir, uint8_t p2) {
    e->_model = &(model_zombie);
    e->_texture = 18;
    e->_speed = 0;
    e->_attack_distance = 350;
    e->_health = 60;


    e->_state_collection = (enemy_state_collection_t) {
        .num_states = _ENEMY_STATE_NULL,
        .states = zombie_enemy_states
    };

    e->_animation_collection = (animation_collection_t) {
        .animations = zombie_animations,
        .num_animations = sizeof(zombie_animations)/sizeof(zombie_animations[0]),
    };

    e->_set_state(e, e->_state);
}

void entity_enemy_zombie_receive_damage(entity_t * e, entity_t * from, int32_t amount) {
    // Ignore damage that's not large enough to gib us
    if (amount > 60) {
        entity_enemy_receive_damage(e, from, amount);
    }
}

void entity_enemy_zombie_attack(entity_t * e) {
    e->_play_sound(e, sfx_enemy_hit);
    // e->_spawn_projectile(e, entity_projectile_gib_constructor, 600, 0, -0.5);
    e->_spawn_projectile_ng(e, ENTITY_ID_PROJECTILE_GIB, 600, 0, -0.5);
}
