
#include "entity_projectile_gib.h"
#include "entity.h"
#include "game.h"
#include "math.h"
#include "audio.h"

void entity_projectile_gib_init(entity_t * e);
void entity_projectile_gib_update(entity_t * e);
void entity_projectile_gib_did_collide(entity_t * e, int axis);
void entity_projectile_gib_did_collide_with_entity(entity_t * e, entity_t * other);

void entity_projectile_gib_constructor(entity_t * e) {
    entity_constructor(e);

    e->_update = entity_projectile_gib_update;
    e->_did_collide = entity_projectile_gib_did_collide;
    e->_did_collide_with_entity = entity_projectile_gib_did_collide_with_entity;

    entity_projectile_gib_init(e);
}

void entity_projectile_gib_init(entity_t * e) {
    e->_bounciness = 0;
    e->_expires = true;
    e->_die_at = game_time + 2;

    e->_yaw = randf();
    e->_pitch = randf();

    entity_set_model(e);
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