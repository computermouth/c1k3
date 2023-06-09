
#include "entity_projectile_nail.h"
#include "entity.h"
#include "game.h"
#include "math.h"
#include "entity_light.h"
#include "audio.h"

void entity_projectile_nail_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_projectile_nail_update(entity_t * e);
void entity_projectile_nail_did_collide(entity_t * e, int axis);
void entity_projectile_nail_did_collide_with_entity(entity_t * e, entity_t * other);

void entity_projectile_nail_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_constructor(e, pos, p1, p2);

    // todo, these casts kinda suck
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_projectile_nail_init;
    e->_update = (void (*)(void *))entity_projectile_nail_update;
    e->_did_collide = (void (*)(void *, int))entity_projectile_nail_did_collide;
    e->_did_collide_with_entity = (void (*)(void *, void *))entity_projectile_nail_did_collide_with_entity;

    // todo, kinda goofy paradigm to set the callback, immediately invoke
    // then never call again. could just combine constructor and init I think
    e->_init(e, p1, p2);
}

void entity_projectile_nail_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_texture = 2;
    e->_model = &model_nail;
    e->_gravity = 0;
    e->_expires = true;
    e->_die_at = game_time + 3;
}

void entity_projectile_nail_update(entity_t * e) {
    e->_update_physics(e);
    e->_draw_model(e);
}

void entity_projectile_nail_did_collide(entity_t * e, int axis) {
    e->_kill(e);
    e->_play_sound(e, sfx_nailgun_hit);
    e->_spawn_particles(e, 2, 80, &model_explosion, 8, 0.4);
    entity_t * tmp_light = game_spawn(entity_light_constructor, e->p, 1, 0xff);
    tmp_light->_expires = true;
    tmp_light->_die_at = game_time + 0.1;
}

void entity_projectile_nail_did_collide_with_entity(entity_t * e, entity_t * other) {
    e->_kill(e);
    other->_receive_damage(other, e, 9);
}