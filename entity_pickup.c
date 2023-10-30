
#include "entity_pickup.h"
#include "game.h"

void entity_pickup_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_pickup_update(entity_t * e);

void entity_pickup_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_constructor(e, pos, p1, p2);
    e->_update = entity_pickup_update;
    entity_pickup_init(e, p1, p2);
}

void entity_pickup_init(entity_t * e, uint8_t p1, uint8_t p2) {
    // todo, investigate size here
    e->s = vec3(12,12,12);
}

void entity_pickup_update(entity_t * e) {

    if (!e->_on_ground) {
        e->_update_physics(e);
    }
    e->_draw_model(e);

    // check to make sure player is alive and game isn't over
    if (game_entity_player != NULL && vec3_dist(e->p, game_entity_player->p) < 40) {
        e->_pickup(e);
    }
}