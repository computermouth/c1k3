
#include "entity_pickup_nailgun.h"
#include "entity.h"
#include "entity_pickup.h"
#include "game.h"
#include "weapon.h"
#include "audio.h"

void entity_pickup_nailgun_init(entity_t * e);
void entity_pickup_nailgun_update(entity_t * e);
void entity_pickup_nailgun_pickup(entity_t * e);
void entity_pickup_nailgun_draw_model(entity_t * e);

void entity_pickup_nailgun_constructor(entity_t *e) {
    entity_pickup_constructor(e);
    e->_update = entity_pickup_nailgun_update;
    e->_pickup = entity_pickup_nailgun_pickup;
    entity_pickup_nailgun_init(e);
}

void entity_pickup_nailgun_init(entity_t * e) {
    entity_set_model(e);
}

void entity_pickup_nailgun_update(entity_t * e) {
    e->_yaw += 0.02;
    entity_pickup_update(e);
}

void entity_pickup_nailgun_pickup(entity_t * e) {
    audio_play(sfx_pickup);

    game_entity_player->_weapons[WEAPON_NAILGUN] = weapon_nailgun_constructor();
    game_entity_player->_weapon_index = WEAPON_NAILGUN;

    e->_kill(e);
}