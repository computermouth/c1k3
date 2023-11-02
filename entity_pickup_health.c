
#include "entity_pickup_health.h"
#include "entity_pickup.h"
#include "game.h"
#include "audio.h"

void entity_pickup_health_init(entity_t * e);
void entity_pickup_health_pickup(entity_t * e);

void entity_pickup_health_constructor(entity_t * e) {
    entity_pickup_constructor(e);
    e->_pickup = entity_pickup_health_pickup;
    entity_pickup_health_init(e);
}

void entity_pickup_health_init(entity_t * e) {
    entity_set_model(e);
}

void entity_pickup_health_pickup(entity_t * e) {
    game_entity_player->_health += 25;
    audio_play(sfx_pickup);
    e->_kill(e);
}