
#include "entity_enemy.h"
#include "entity_projectile_plasma.h"

#include "audio.h"

animation_t hound_animations[] = {
    {   // 0: Idle
        .time = 1,
        .num_frames = 1,
        // .frames = (uint32_t[]){0, 0},
        .frames_ng = (animation_frame_t[]) {
            {.name = "default"},
        },
    },
    {   // 2: Run
        .time = 0.15f,
        .num_frames = 2,
        .frames_ng = (animation_frame_t[]) {
            {.name = "run1"},
            {.name = "run2"},
        },
    },
    {   // 2: Run
        .time = 0.15f,
        .num_frames = 2,
        .frames_ng = (animation_frame_t[]) {
            {.name = "run1"},
            {.name = "run2"},
        },
    },
    {   // 3: Attack prepare
        .time = 1,
        .num_frames = 1,
        .frames_ng = (animation_frame_t[]) {
            {.name = "run1"},
        },
    },
    {   // 4: Attack
        .time = 0.1f,
        .num_frames = 7,
        .frames_ng = (animation_frame_t[]) {
            {.name = "run1"},
            {.name = "run2"},
            {.name = "run2"},
            {.name = "run2"},
            {.name = "default"},
            {.name = "default"},
            {.name = "default"},
        },
    },
};

// hack for caching parsed frame names per-map
static ref_entt_t * last_ref_entt = NULL;

enemy_state_t hound_enemy_states[_ENEMY_STATE_NULL] = {
    {ENEMY_ANIMATION_IDLE,   0, 0.1, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_WALK, 0.2, 0.5, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_RUN,   1, 0.3, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_IDLE,   0, 0.5, ENEMY_STATE_FOLLOW},
    {ENEMY_ANIMATION_ATTACK,   0,   1, ENEMY_STATE_ATTACK_RECOVER},
    {ENEMY_ANIMATION_ATTACK_PREPARE,   0, 0.0, ENEMY_STATE_ATTACK_EXEC},
    {ENEMY_ANIMATION_IDLE,   0, 0.0, ENEMY_STATE_ATTACK_PREPARE},
    {ENEMY_ANIMATION_RUN,   1, 0.3, ENEMY_STATE_ATTACK_AIM},
};

void entity_enemy_hound_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_enemy_hound_did_collide_with_entity(entity_t * e, entity_t * other);
void entity_enemy_hound_attack(entity_t * e);

void entity_enemy_hound_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_enemy_constructor(e, pos, p1, p2);
    e->_init = entity_enemy_hound_init;
    e->_did_collide_with_entity = entity_enemy_hound_did_collide_with_entity;
    e->_attack = entity_enemy_hound_attack;
    e->_init(e, p1, p2);

    // todo, move everything from here on to grunt_init
    entity_parse_animation_frames(
        e->_params->entity_generic_params.ref_entt,
        hound_animations,
        sizeof(hound_animations)/sizeof(hound_animations[0]),
        last_ref_entt
    );

    e->_texture = e->_params->entity_generic_params.ref_entt->tex_id;
    vector * frames = e->_params->entity_generic_params.ref_entt->frames;
    uint32_t * uframes = vector_begin(frames);
    e->_model->frames = uframes;
    e->_model->nv = e->_params->entity_generic_params.ref_entt->vert_len;
    e->s = e->_params->entity_generic_params.ref_entt->size;
}

void entity_enemy_hound_init(entity_t * e, uint8_t patrol_dir, uint8_t p2) {
    e->_model = &(model_hound);
    e->_texture = 22;
    e->_health = 25;
    e->_check_against = ENTITY_GROUP_PLAYER;

    e->s = vec3(12,16,12);

    e->_attack_distance = 200;
    e->_evade_distance = 64;
    e->_attack_chance = 0.7;
    e->_speed = 256;

    e->_state_collection = (enemy_state_collection_t) {
        .num_states = _ENEMY_STATE_NULL,
        .states = hound_enemy_states
    };

    e->_animation_collection = (animation_collection_t) {
        .animations = hound_animations,
        .num_animations = sizeof(hound_animations)/sizeof(hound_animations[0]),
    };

    e->_set_state(e, e->_state);
}

void entity_enemy_hound_did_collide_with_entity(entity_t * e, entity_t * other) {
    if (!e->_did_hit && e->_state == e->_STATE_ATTACK_EXEC) {
        e->_did_hit = 1;
        other->_receive_damage(other, e, 14);
    }
}

void entity_enemy_hound_attack(entity_t * e) {
    e->_play_sound(e, sfx_enemy_hound_attack);
    e->v = vec3_rotate_y(vec3(0, 250, 600), e->_target_yaw);
    e->_on_ground = 0;
    e->_did_hit = 0;

    // Ignore ledges while attacking
    e->_keep_off_ledges = 0;
    // todo, timeouts
    // clearTimeout(this._reset_ledges);
    // this._reset_ledges = setTimeout(()=>this._keep_off_ledges = 1, 1000);
}