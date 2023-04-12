
#ifndef _GAME_
#define _GAME_

#include "entity.h"
#include <stdint.h>

extern float game_tick;
extern float game_time;
extern entity_collection_t game_entities_friendly;
extern entity_collection_t game_entities_enemies;


void game_init(int map_index);
entity_t game_spawn (entity_t (*func)(), vec3_t pos, uint8_t p1, uint8_t p2);
void game_run(float time_now);

#endif