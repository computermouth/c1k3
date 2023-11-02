
#include "entity_projectile_shell.h"
#include "entity.h"
#include "game.h"
#include "map.h"
#include "math.h"
#include "entity_light.h"

void entity_projectile_shell_init(entity_t * e);
void entity_projectile_shell_update(entity_t * e);
void entity_projectile_shell_did_collide(entity_t * e, int axis);
void entity_projectile_shell_did_collide_with_entity(entity_t * e, entity_t * other);

void entity_projectile_shell_constructor(entity_t * e) {
    entity_constructor(e);

    e->_init = entity_projectile_shell_init;
    e->_update = entity_projectile_shell_update;
    e->_did_collide = entity_projectile_shell_did_collide;
    e->_did_collide_with_entity = entity_projectile_shell_did_collide_with_entity;

    /* shells are invis
    *  so no model setup */

    e->_init(e);
}

void entity_projectile_shell_init(entity_t * e) {
    e->_gravity = 0;
    e->_expires = true;
    e->_die_at = game_time + 0.1;
}

void entity_projectile_shell_update(entity_t * e) {
    e->_update_physics(e);
}

void entity_projectile_shell_did_collide(entity_t * e, int axis) {
    e->_kill(e);
    e->_spawn_particles(e, 2, 80, ENTITY_ID_PARTICLE_SLUG, 0.4);

    entity_params_t l = {
        .id = ENTITY_ID_LIGHT,
        .position = e->p,
        .entity_light_params = {
            .rgba[0] = 0xff,
            .rgba[1] = 0xff,
            .rgba[2] = 0xff,
            .rgba[3] = 0x01,
        },
    };
    entity_t * tmp_light = game_spawn_ng(&l);

    tmp_light->_expires = true;
    tmp_light->_die_at = game_time + 0.1;
}

void entity_projectile_shell_did_collide_with_entity(entity_t * e, entity_t * other) {
    e->_kill(e);
    other->_receive_damage(other, e, 4);
}