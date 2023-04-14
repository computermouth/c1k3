
#include "entity_pickup_health.h"
#include "entity_pickup.h"
#include "game.h"

void entity_pickup_health_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_pickup_health_pickup(entity_t * e);

void entity_pickup_health_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_pickup_constructor(e, pos, p1, p2);
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_pickup_health_init;
    e->_pickup = (void (*)(void *))entity_pickup_health_pickup;
    e->_init(e, p1, p2);
}

void entity_pickup_health_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_texture = 23;
}

void entity_pickup_health_pickup(entity_t * e) {
    game_entity_player->_health += 25;
    // todo, audio_play(sfx_pickup)
    e->_kill(e);
}