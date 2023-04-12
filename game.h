
#ifndef _GAME_
#define _GAME_

#include "entity.h"
#include <stdint.h>

extern float game_tick;
extern float game_time;
extern entity_ref_collection_t game_entities_friendly;
extern entity_ref_collection_t game_entities_enemies;
extern entity_t * game_entity_player;
extern int game_map_index;

void game_init(int map_index);
entity_t * game_spawn (void (*func)(), vec3_t pos, uint8_t p1, uint8_t p2);
void game_run(float time_now);
void title_show_message(char *txt, char *sub);

#endif