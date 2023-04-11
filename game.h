
#ifndef _GAME_
#define _GAME_

#include "entity.h"

extern float game_tick;
extern float game_time;
extern entity_collection_t game_entities_friendly;
extern entity_collection_t game_entities_enemies;

entity_t game_spawn (entity_t (*func)(vec3_t pos, vec3_t p1, vec3_t p2), vec3_t pos, vec3_t p1, vec3_t p2);

#endif