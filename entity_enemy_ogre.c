
#include "entity_enemy.h"
#include "entity_projectile_grenade.h"
#include "audio.h"

animation_t ogre_animations[] = {
    {   // 0: Idle
        .time = 1,
        .num_frames = 1,
        .frames = (uint32_t[]){0},
    },
    {   // 1: Walk
        .time = 0.80f,
        .num_frames = 4,
        .frames = (uint32_t[]){1,2,3,4},
    },
    {   // 2: Run
        .time = 0.40f,
        .num_frames = 4,
        .frames = (uint32_t[]){1,2,3,4},
    },
    {   // 3: Attack prepare
        .time = 0.35f,
        .num_frames = 4,
        .frames = (uint32_t[]){0,5,5,5},
    },
    {   // 4: Attack
        .time = 0.35f,
        .num_frames = 4,
        .frames = (uint32_t[]){5,0,0,0},
    },
};

void entity_enemy_ogre_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_enemy_ogre_attack(entity_t * e);

void entity_enemy_ogre_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2, entity_params_t * ep) {
    entity_enemy_constructor(e, pos, p1, p2, ep);
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_enemy_ogre_init;
    e->_attack = (void (*)(void *))entity_enemy_ogre_attack;
    e->_init(e, p1, p2);
}

void entity_enemy_ogre_init(entity_t * e, uint8_t patrol_dir, uint8_t p2) {
    e->_model = &(model_ogre);
    e->_texture = 20;
    e->_speed = 96;
    e->_health = 200;
    e->s = vec3(14,36,14);

    e->_attack_distance = 350;
    e->_animation_collection = (animation_collection_t) {
        .animations = ogre_animations,
        .num_animations = sizeof(ogre_animations)/sizeof(ogre_animations[0]),
    };
}

void entity_enemy_ogre_attack(entity_t * e) {
    e->_play_sound(e, sfx_grenade_shoot);
    entity_t * t = e->_spawn_projectile(e, (void (*)(void *, vec3_t, uint8_t, uint8_t))entity_projectile_grenade_constructor, 600, 0, -0.4);
    t->_damage = 40;
}