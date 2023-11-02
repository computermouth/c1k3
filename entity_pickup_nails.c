
#include "entity_pickup_nails.h"
#include "entity_pickup.h"
#include "game.h"
#include "weapon.h"
#include "audio.h"

void entity_pickup_nails_init(entity_t * e);
void entity_pickup_nail_pickup(entity_t * e);

void entity_pickup_nails_constructor(entity_t * e) {
    entity_pickup_constructor(e);
    e->_pickup = entity_pickup_nail_pickup;
    entity_pickup_nails_init(e);
}

void entity_pickup_nails_init(entity_t * e) {
    entity_set_model(e);
}

void entity_pickup_nail_pickup(entity_t * e) {
    game_entity_player->_weapons[WEAPON_NAILGUN]._ammo += 50;
    audio_play(sfx_pickup);
    e->_kill(e);
}