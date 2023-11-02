
#include "entity_pickup_grenades.h"
#include "entity_pickup.h"
#include "game.h"
#include "weapon.h"
#include "audio.h"

void entity_pickup_grenades_init(entity_t * e);
void entity_pickup_grenades_pickup(entity_t * e);

void entity_pickup_grenades_constructor(entity_t * e) {
    entity_pickup_constructor(e);
    e->_pickup = entity_pickup_grenades_pickup;
    entity_pickup_grenades_init(e);
}

void entity_pickup_grenades_init(entity_t * e) {
    entity_set_model(e);
}

void entity_pickup_grenades_pickup(entity_t * e) {
    game_entity_player->_weapons[WEAPON_GRENADELAUNCHER]._ammo += 10;
    audio_play(sfx_pickup);
    e->_kill(e);
}