
#include "entity_pickup_grenadelauncher.h"
#include "entity_pickup.h"
#include "game.h"
#include "weapon.h"
#include "audio.h"

void entity_pickup_grenadelauncher_init(entity_t * e);
void entity_pickup_grenadelauncher_update(entity_t * e);
void entity_pickup_grenadelauncher_pickup(entity_t * e);

void entity_pickup_grenadelauncher_constructor(entity_t * e) {
    entity_pickup_constructor(e);
    e->_update = entity_pickup_grenadelauncher_update;
    e->_pickup = entity_pickup_grenadelauncher_pickup;
    entity_pickup_grenadelauncher_init(e);
}

void entity_pickup_grenadelauncher_init(entity_t * e) {
    entity_set_model(e);
}

void entity_pickup_grenadelauncher_update(entity_t * e) {
    e->_yaw += 0.02;
    entity_pickup_update(e);
}

void entity_pickup_grenadelauncher_pickup(entity_t * e) {
    audio_play(sfx_pickup);

    game_entity_player->_weapons[WEAPON_GRENADELAUNCHER] = weapon_grenadelauncher_constructor();
    game_entity_player->_weapon_index = WEAPON_GRENADELAUNCHER;

    e->_kill(e);
}