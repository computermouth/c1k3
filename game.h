
#ifndef _GAME_
#define _GAME_

#include "entity.h"
#include <stdint.h>

typedef enum {
    MENU_STATE,
    GAME_STATE
} state_t;

extern state_t game_state;
extern float game_tick;
extern float game_time;
extern vector * game_entities_list_all;
extern vector * game_entities_list_friendly;
extern vector * game_entities_list_enemies;
extern entity_t * game_entity_player;
extern int game_map_index;

void game_free_entities();
void game_entities_enemies_push(entity_t ** e);
void game_entities_enemies_pop(entity_t ** e);
void game_entities_friendly_push(entity_t ** e);
void game_entities_friendly_pop(entity_t ** e);
void game_init(int map_index);
entity_t * game_spawn (void (*func)(entity_t *, vec3_t, uint8_t, uint8_t), vec3_t pos, uint8_t p1, uint8_t p2);
void game_run(float time_now);
void game_next_level();

#endif