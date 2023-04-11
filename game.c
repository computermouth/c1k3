
#include <stdlib.h>

#include "entity.h"
#include "game.h"

float game_tick = 0.0f;
float game_time = 0.016f;
entity_collection_t game_entities = {0};
entity_collection_t game_entities_friendly = {0};
entity_collection_t game_entities_enemies = {0};

// todo, is this fuckin goofy?
// pos -> game_spawn -> constructor -> init -> constructor -> init
entity_t game_spawn (entity_t (*func)(vec3_t pos, vec3_t p1, vec3_t p2), vec3_t pos, vec3_t p1, vec3_t p2) {
    entity_t tmp = func(pos, p1, p2);
    game_entities.length++;
    game_entities.entities = realloc(game_entities.entities, sizeof(entity_t) * game_entities.length);
    game_entities.entities[game_entities.length - 1] = tmp;
    return tmp;
}