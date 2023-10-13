
#include "entity_projectile_shell.h"
#include "entity.h"
#include "game.h"
#include "math.h"
#include "entity_light.h"

void entity_projectile_shell_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_projectile_shell_update(entity_t * e);
void entity_projectile_shell_did_collide(entity_t * e, int axis);
void entity_projectile_shell_did_collide_with_entity(entity_t * e, entity_t * other);

void entity_projectile_shell_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_constructor(e, pos, p1, p2);

    // todo, these casts kinda suck
    e->_init = entity_projectile_shell_init;
    e->_update = entity_projectile_shell_update;
    e->_did_collide = entity_projectile_shell_did_collide;
    e->_did_collide_with_entity = entity_projectile_shell_did_collide_with_entity;

    // todo, kinda goofy paradigm to set the callback, immediately invoke
    // then never call again. could just combine constructor and init I think
    e->_init(e, p1, p2);
}

void entity_projectile_shell_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_gravity = 0;
    e->_expires = true;
    e->_die_at = game_time + 0.1;
}

void entity_projectile_shell_update(entity_t * e) {
    e->_update_physics(e);
}

void entity_projectile_shell_did_collide(entity_t * e, int axis) {
    e->_kill(e);
    e->_spawn_particles(e, 2, 80, &model_explosion, 4, 0.4);
    // todo, change parameter types to float, fuck it
    // entity_t * tmp_light = game_spawn(entity_light_constructor, e->p, 0.5, 0xff);
    entity_t * tmp_light = game_spawn(entity_light_constructor, e->p, 1, 0xff, NULL);
    tmp_light->_expires = true;
    tmp_light->_die_at = game_time + 0.1;
}

void entity_projectile_shell_did_collide_with_entity(entity_t * e, entity_t * other) {
    e->_kill(e);
    other->_receive_damage(other, e, 4);
}