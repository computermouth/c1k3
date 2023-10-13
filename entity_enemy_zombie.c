
#include "entity.h"
#include "entity_enemy.h"
#include "entity_projectile_gib.h"
#include "audio.h"

animation_t zombie_animations[] = {
    {   // 0: Idle
        .time = 1,
        .num_frames = 1,
        .frames = (uint32_t[]){0},
    },
    {   // 1: Walk
        .time = 0.40f,
        .num_frames = 4,
        .frames = (uint32_t[]){1,2,3,4},
    },
    {   // 2: Run
        .time = 0.20f,
        .num_frames = 4,
        .frames = (uint32_t[]){1,2,3,4},
    },
    {   // 3: Attack prepare
        .time = 0.25f,
        .num_frames = 4,
        .frames = (uint32_t[]){0,0,5,5},
    },
    {   // 4: Attack
        .time = 0.25f,
        .num_frames = 4,
        .frames = (uint32_t[]){5,0,0,0},
    },
};

enemy_state_t zombie_enemy_states[_ENEMY_STATE_NULL] = {
    {ENEMY_ANIMATION_IDLE,   0, 0.1, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_WALK, 0.5, 0.5, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_IDLE,   0, 0.1, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_IDLE,   0, 1.1, ENEMY_STATE_IDLE},
    {ENEMY_ANIMATION_ATTACK,   0, 0.4, ENEMY_STATE_ATTACK_RECOVER},
    {ENEMY_ANIMATION_ATTACK_PREPARE,   0, 0.4, ENEMY_STATE_ATTACK_EXEC},
    {ENEMY_ANIMATION_IDLE,   0, 0.1, ENEMY_STATE_ATTACK_PREPARE},
    {ENEMY_ANIMATION_IDLE,   0, 0.1, ENEMY_STATE_ATTACK_AIM},
};

void entity_enemy_zombie_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_enemy_zombie_receive_damage(entity_t * e, entity_t *from, int32_t amount);
void entity_enemy_zombie_attack(entity_t * e);

void entity_enemy_zombie_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_enemy_constructor(e, pos, p1, p2);
    e->_init = entity_enemy_zombie_init;
    e->_receive_damage = (void (*)(void *, void *, int32_t))entity_enemy_zombie_receive_damage;
    e->_attack = (void (*)(void *))entity_enemy_zombie_attack;
    e->_init(e, p1, p2);
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

    e->_set_state(e, e->_STATE_IDLE);
}

void entity_enemy_zombie_receive_damage(entity_t * e, entity_t * from, int32_t amount) {
    // Ignore damage that's not large enough to gib us
    if (amount > 60) {
        entity_enemy_receive_damage(e, from, amount);
    }
}

void entity_enemy_zombie_attack(entity_t * e) {
    e->_play_sound(e, sfx_enemy_hit);
    e->_spawn_projectile(e, (void (*)(void *, vec3_t, uint8_t, uint8_t))entity_projectile_gib_constructor, 600, 0, -0.5);
}