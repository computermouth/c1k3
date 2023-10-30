
#include "entity_pickup_health.h"
#include "entity_pickup.h"
#include "game.h"
#include "audio.h"

void entity_pickup_health_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_pickup_health_pickup(entity_t * e);

void entity_pickup_health_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_pickup_constructor(e, pos, p1, p2);
    e->_pickup = entity_pickup_health_pickup;
    entity_pickup_health_init(e, p1, p2);
}

void entity_pickup_health_init(entity_t * e, uint8_t p1, uint8_t p2) {
    entity_set_model(e);
}

void entity_pickup_health_pickup(entity_t * e) {
    game_entity_player->_health += 25;
    audio_play(sfx_pickup);
    e->_kill(e);
}