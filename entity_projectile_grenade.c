
#include <math.h>

#include "entity.h"
#include "game.h"
#include "math.h"
#include "render.h"
#include "entity_light.h"
#include "audio.h"

void entity_projectile_grenade_init(entity_t * e);
void entity_projectile_grenade_update(entity_t * e);
void entity_projectile_grenade_did_collide(entity_t * e, int axis);
void entity_projectile_grenade_did_collide_with_entity(entity_t * e, entity_t * other);
void entity_projectile_grenade_kill(entity_t * e);

void entity_projectile_grenade_constructor(entity_t * e) {
    entity_constructor(e);

    e->_update = entity_projectile_grenade_update;
    e->_did_collide = entity_projectile_grenade_did_collide;
    e->_did_collide_with_entity = entity_projectile_grenade_did_collide_with_entity;
    e->_kill = entity_projectile_grenade_kill;
    entity_projectile_grenade_init(e);
}

void entity_projectile_grenade_init(entity_t * e) {
    e->_expires = true;
    e->_die_at = game_time + 2;
    e->_bounciness = 0.5;
    e->_damage = 120;
    entity_set_model(e);
}

void entity_projectile_grenade_update(entity_t * e) {
    entity_update_physics(e);

    // roll em
    e->_yaw += (fabs(e->v.x) + fabs(e->v.y) + fabs(e->v.z)) * .02 * game_tick;
    e->_pitch += (fabs(e->v.x) + fabs(e->v.y) + fabs(e->v.z)) * .02 * game_tick;

    e->_draw_model(e);
    r_push_light(vec3_add(e->p, vec3(0,16,0)), (sinf(game_time*10)+2)*5, 255, 32, 0);
    e->f = e->_on_ground ? 5 : 0.5;
}

void entity_projectile_grenade_did_collide(entity_t * e, int axis) {
    if (axis != 1 || e->v.y < -128) {
        e->_yaw += randf();
        e->_play_sound(e, sfx_grenade_bounce);
    }
}

void entity_projectile_grenade_did_collide_with_entity(entity_t * e, entity_t * other) {
    // silence unused
    other = other;

    e->_kill(e);
}

void entity_projectile_grenade_kill(entity_t * e) {
    // Deal some damage to nearby entities
    vector * to_check = e->_check_entities;
    uint32_t len = vector_size(to_check);
    for (uint32_t i = 0; i < len; i++) {
        entity_t ** other_p = vector_at(to_check, i);
        entity_t * other = *other_p;
        float dist = vec3_dist(e->p, other->p);
        if (dist < 196)
            other->_receive_damage(other, e, scale(dist, 0, 196, e->_damage, 0));
    }

    entity_kill(e);
    e->_play_sound(e, sfx_grenade_explode);
    e->_spawn_particles(e, 2, 800, ENTITY_ID_PARTICLE_SLUG, 1);

    entity_params_t l = {
        .id = ENTITY_ID_LIGHT,
        .position = vec3_add(e->p, vec3(0,16,0)),
        .entity_light_params = {
            .rgba[0] = 0xE0,
            .rgba[1] = 0x60,
            .rgba[2] = 0x80,
            .rgba[3] = 0xFF,
        },
    };
    entity_t * tmplight = game_spawn(&l);

    tmplight->_expires = true;
    tmplight->_die_at = game_time + 0.2;
}