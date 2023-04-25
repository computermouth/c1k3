
#include "entity_enemy.h"
#include "entity_projectile_plasma.h"

#include "audio.h"

void entity_enemy_enforcer_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_enemy_enforcer_attack(entity_t * e);

void entity_enemy_enforcer_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_enemy_constructor(e, pos, p1, p2);
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_enemy_enforcer_init;
    e->_attack = (void (*)(void *))entity_enemy_enforcer_attack;
    e->_init(e, p1, p2);
}

void entity_enemy_enforcer_init(entity_t * e, uint8_t patrol_dir, uint8_t p2) {
    e->_model = &(model_enforcer);
    e->_texture = 19;
    e->_health = 80;
    e->s = vec3(14,44,14);
}

void entity_enemy_enforcer_attack(entity_t * e) {
    e->_play_sound(e, sfx_plasma_shoot);
    e->_spawn_projectile(e, (void (*)(void *, vec3_t, uint8_t, uint8_t))entity_projectile_plasma_constructor, 800, 0, 0);
}