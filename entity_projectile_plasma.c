
#include "entity_projectile_plasma.h"
#include "entity.h"
#include "entity_light.h"
#include "game.h"
#include "math.h"
#include "render.h"
#include "audio.h"

void entity_projectile_plasma_init(entity_t * e);
void entity_projectile_plasma_update(entity_t * e);
void entity_projectile_plasma_did_collide(entity_t * e, int axis);
void entity_projectile_plasma_did_collide_with_entity(entity_t * e, entity_t * other);
void entity_projectile_plasma_draw_model(entity_t * e);

void entity_projectile_plasma_constructor(entity_t * e) {
    entity_constructor(e);

    e->_update = entity_projectile_plasma_update;
    e->_did_collide = entity_projectile_plasma_did_collide;
    e->_did_collide_with_entity = entity_projectile_plasma_did_collide_with_entity;
    e->_draw_model = entity_projectile_plasma_draw_model;

    entity_projectile_plasma_init(e);
}

void entity_projectile_plasma_init(entity_t * e) {
    e->_gravity = 0;
    e->_expires = true;
    e->_die_at = game_time + 3;

    entity_set_model(e);
}

void entity_projectile_plasma_update(entity_t * e) {
    e->_update_physics(e);
    e->_yaw += 5.0 * game_tick;
    e->_draw_model(e);
    r_push_light(e->p, 5, 255, 128, 0);
}

void entity_projectile_plasma_did_collide(entity_t * e, int axis) {
    // silence Wunused
    axis = axis;

    e->_kill(e);
    e->_play_sound(e, sfx_nailgun_hit);
    e->_spawn_particles(e, 2, 80, ENTITY_ID_PARTICLE_SLUG, 0.4);

    entity_params_t l = {
        .id = ENTITY_ID_LIGHT,
        .position = vec3_add(e->p, vec3(0,10,0)),
        .entity_light_params = {
            .rgba[0] = 0xff,
            .rgba[1] = 0xff,
            .rgba[2] = 0xff,
            .rgba[3] = 0x05,
        },
    };
    entity_t * tmplight = game_spawn(&l);

    tmplight->_expires = true;
    tmplight->_die_at = game_time + 0.1;
}

void entity_projectile_plasma_did_collide_with_entity(entity_t * e, entity_t * other) {
    e->_kill(e);
    other->_receive_damage(other, e, 15);
}

void entity_projectile_plasma_draw_model(entity_t * e) {
    draw_call_t call = {
        .pos = e->p,
        .yaw = e->_yaw,
        .pitch = e->_pitch,
        .texture = e->_texture,
        .f1 = e->_model.frames[0],
        .f2 = e->_model.frames[0],
        .mix = 0.0f,
        .unlit = 1,
        .num_verts = e->_model.nv
    };
    r_draw(call);
}