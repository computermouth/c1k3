
#include "entity_pickup_nails.h"
#include "entity_pickup.h"
#include "game.h"
#include "weapon.h"
#include "audio.h"

void entity_pickup_nails_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_pickup_nail_pickup(entity_t * e);

void entity_pickup_nails_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_pickup_constructor(e, pos, p1, p2);
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_pickup_nails_init;
    e->_pickup = (void (*)(void *))entity_pickup_nail_pickup;
    e->_init(e, p1, p2);
}

void entity_pickup_nails_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_texture = 24;
}

void entity_pickup_nail_pickup(entity_t * e) {
    game_entity_player->_weapons[WEAPON_NAILGUN]._ammo += 50;
    audio_play(sfx_pickup);
    e->_kill(e);
}