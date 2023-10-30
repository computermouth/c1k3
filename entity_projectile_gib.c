
#include "entity_projectile_gib.h"
#include "entity.h"
#include "game.h"
#include "math.h"
#include "audio.h"

void entity_projectile_gib_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_projectile_gib_update(entity_t * e);
void entity_projectile_gib_did_collide(entity_t * e, int axis);
void entity_projectile_gib_did_collide_with_entity(entity_t * e, entity_t * other);

void entity_projectile_gib_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_constructor(e, pos, p1, p2);

    // todo, these casts kinda suck
    e->_init = entity_projectile_gib_init;
    e->_update = entity_projectile_gib_update;
    e->_did_collide = entity_projectile_gib_did_collide;
    e->_did_collide_with_entity = entity_projectile_gib_did_collide_with_entity;

    // todo, kinda goofy paradigm to set the callback, immediately invoke
    // then never call again. could just combine constructor and init I think
    e->_init(e, p1, p2);

    e->_texture = e->_params->entity_generic_params.ref_entt->tex_id;
    vector * frames = e->_params->entity_generic_params.ref_entt->frames;
    uint32_t * uframes = vector_begin(frames);
    e->_model.frames = uframes;
    e->_model.nv = e->_params->entity_generic_params.ref_entt->vert_len;
    e->s = e->_params->entity_generic_params.ref_entt->size;
}

void entity_projectile_gib_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_texture = 18;
    e->_bounciness = 0;
    e->_expires = true;
    e->_die_at = game_time + 2;
    // e->_model = &model_gib;

    e->_yaw = randf();
    e->_pitch = randf();
}

void entity_projectile_gib_update(entity_t * e) {
    entity_update_physics(e);
    e->_draw_model(e);
    e->f = e->_on_ground ? 15 : 0;
}

void entity_projectile_gib_did_collide(entity_t * e, int axis) {
    if (axis == 1 && e->v.y < -128) {
        e->_play_sound(e, sfx_enemy_hit);
    }
}

void entity_projectile_gib_did_collide_with_entity(entity_t * e, entity_t * other) {
    other->_receive_damage(other, e, 10);
    e->_kill(e);
}