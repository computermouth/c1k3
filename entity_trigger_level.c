
#include "entity_projectile_gib.h"
#include "entity.h"
#include "game.h"
#include "math.h"

void entity_trigger_level_update(entity_t * e);

void entity_trigger_level_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_constructor(e, pos, p1, p2);
    e->_update = entity_trigger_level_update;
    e->_init(e, p1, p2);

    char * str_p1 = entity_param_lookup("range", e->_params->entity_generic_params.extras);
    if (str_p1)
        e->_trigger_range = atoi(str_p1);
    else
        e->_trigger_range = 64; // two blocks?
}

void entity_trigger_level_update(entity_t * e) {
    if (game_entity_player && !e->_dead && vec3_dist(e->p, game_entity_player->p) < e->_trigger_range) {
        game_next_level();
        e->_dead = 1;
    }
}