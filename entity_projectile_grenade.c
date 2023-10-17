
#include <math.h>

#include "entity_projectile_gib.h"
#include "entity.h"
#include "game.h"
#include "math.h"
#include "render.h"
#include "entity_light.h"
#include "audio.h"

void entity_projectile_grenade_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_projectile_grenade_update(entity_t * e);
void entity_projectile_grenade_did_collide(entity_t * e, int axis);
void entity_projectile_grenade_did_collide_with_entity(entity_t * e, entity_t * other);
void entity_projectile_grenade_kill(entity_t * e);

void entity_projectile_grenade_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_constructor(e, pos, p1, p2);

    e->_init = entity_projectile_grenade_init;
    e->_update = entity_projectile_grenade_update;
    e->_did_collide = entity_projectile_grenade_did_collide;
    e->_did_collide_with_entity = entity_projectile_grenade_did_collide_with_entity;
    e->_kill = entity_projectile_grenade_kill;
    e->_init(e, p1, p2);

    // todo, remove when weapons use spawn_projectile_ng
    if (! e->_params)
        return;

    e->_texture = e->_params->entity_generic_params.ref_entt->tex_id;
    vector * frames = e->_params->entity_generic_params.ref_entt->frames;
    uint32_t * uframes = vector_begin(frames);
    e->_model->frames = uframes;
    e->_model->nv = e->_params->entity_generic_params.ref_entt->vert_len;
    e->s = e->_params->entity_generic_params.ref_entt->size;
}

void entity_projectile_grenade_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_texture = 8;
    e->_model = &model_grenade;
    e->_expires = true;
    e->_die_at = game_time + 2;
    e->_bounciness = 0.5;
    e->_damage = 120;
}

void entity_projectile_grenade_update(entity_t * e) {
    entity_update_physics(e);

    // roll em
    e->_yaw += (fabs(e->v.x) + fabs(e->v.y) + fabs(e->v.z)) * .02 * game_tick;
    e->_pitch += (fabs(e->v.x) + fabs(e->v.y) + fabs(e->v.z)) * .02 * game_tick;

    e->_draw_model(e);
    r_push_light(vec3_add(e->p, vec3(0,16,0)), (sinf(game_time*10)+2)*0.5, 255, 32, 0);
    e->f = e->_on_ground ? 5 : 0.5;
}

void entity_projectile_grenade_did_collide(entity_t * e, int axis) {
    if (axis != 1 || e->v.y < -128) {
        e->_yaw += randf();
        e->_play_sound(e, sfx_grenade_bounce);
    }
}

void entity_projectile_grenade_did_collide_with_entity(entity_t * e, entity_t * other) {
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
    e->_spawn_particles(e, 20, 800, &model_explosion, 8, 1);
    entity_t * tmplight = game_spawn(entity_light_constructor, vec3_add(e->p, vec3(0,16,0)), 250, 0x08f, NULL);
    tmplight->_expires = true;
    tmplight->_die_at = game_time + 0.2;
}