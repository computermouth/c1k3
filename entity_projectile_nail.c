
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

    e->_update = entity_projectile_nail_update;
    e->_did_collide = entity_projectile_nail_did_collide;
    e->_did_collide_with_entity = entity_projectile_nail_did_collide_with_entity;

    entity_projectile_nail_init(e, p1, p2);
}

void entity_projectile_nail_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_gravity = 0;
    e->_expires = true;
    e->_die_at = game_time + 3;

    entity_set_model(e);
}

void entity_projectile_nail_update(entity_t * e) {
    e->_update_physics(e);
    e->_draw_model(e);
}

void entity_projectile_nail_did_collide(entity_t * e, int axis) {
    e->_kill(e);
    e->_play_sound(e, sfx_nailgun_hit);
    e->_spawn_particles(e, 2, 80, ENTITY_ID_PARTICLE_SLUG, 0.4);

    entity_params_t l = {
        .id = ENTITY_ID_LIGHT,
        .entity_light_params = {
            .position = e->p,
            .rgba[0] = 0xff,
            .rgba[1] = 0xff,
            .rgba[2] = 0xff,
            .rgba[3] = 0x01,
        },
    };
    entity_t * tmplight = game_spawn_ng(&l);

    tmplight->_expires = true;
    tmplight->_die_at = game_time + 0.1;
}

void entity_projectile_nail_did_collide_with_entity(entity_t * e, entity_t * other) {
    e->_kill(e);
    other->_receive_damage(other, e, 9);
}