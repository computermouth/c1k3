
#include "entity_enemy.h"
#include "entity_projectile_shell.h"
#include "entity_light.h"
#include "game.h"
#include "audio.h"

void entity_enemy_grunt_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_enemy_grunt_attack(entity_t * e);

void entity_enemy_grunt_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_enemy_constructor(e, pos, p1, p2);
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_enemy_grunt_init;
    e->_attack = (void (*)(void *))entity_enemy_grunt_attack;
    e->_init(e, p1, p2);
}

void entity_enemy_grunt_init(entity_t * e, uint8_t patrol_dir, uint8_t p2) {
    e->_model = &(model_grunt);
    e->_texture = 17;
    e->_health = 40;
}

void entity_enemy_grunt_attack(entity_t * e) {
    e->_play_sound(e, sfx_shotgun_shoot);
    entity_t * tmplight = game_spawn(entity_light_constructor, vec3_add(e->p, vec3(0,30,0)), 10, 0xff);
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