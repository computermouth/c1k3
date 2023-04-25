
#include "entity_pickup_nailgun.h"
#include "entity.h"
#include "entity_pickup.h"
#include "game.h"
#include "weapon.h"
#include "audio.h"

void entity_pickup_nailgun_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_pickup_nailgun_update(entity_t * e);
void entity_pickup_nailgun_pickup(entity_t * e);
void entity_pickup_nailgun_draw_model(entity_t * e);

void entity_pickup_nailgun_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_pickup_constructor(e, pos, p1, p2);
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_pickup_nailgun_init;
    e->_update = (void (*)(void *))entity_pickup_nailgun_update;
    e->_pickup = (void (*)(void *))entity_pickup_nailgun_pickup;
    e->_init(e, p1, p2);
}

void entity_pickup_nailgun_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_texture = 12;
    e->_model = &model_pickup_nailgun;
}

void entity_pickup_nailgun_update(entity_t * e) {
    e->_yaw += 0.02;
    entity_pickup_update(e);
}

void entity_pickup_nailgun_pickup(entity_t * e) {
    audio_play(sfx_pickup);
    // todo, hate this
    if (game_entity_player->_weapon_length < WEAPON_END) {
        game_entity_player->_weapon_length++;
        game_entity_player->_weapons[game_entity_player->_weapon_length - 1] = weapon_nailgun_constructor();
        game_entity_player->_weapon_index = game_entity_player->_weapon_length - 1;
    }
    e->_kill(e);
}