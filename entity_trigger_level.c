
#include "entity_projectile_gib.h"
#include "entity.h"
#include "game.h"
#include "math.h"

void entity_trigger_level_update(entity_t * e);

void entity_trigger_level_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_constructor(e, pos, p1, p2);
    e->_update = (void (*)(void *))entity_trigger_level_update;
    e->_init(e, p1, p2);
}

// todo, weird???
// why not check collision with player entity
void entity_trigger_level_update(entity_t * e) {
    if (!e->_dead && vec3_dist(e->p, game_entity_player->p) < 64) {
        game_next_level();
        e->_dead = 1;
    }
}