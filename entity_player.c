
#include <stdbool.h>
#include <stdlib.h>

#include "entity_player.h"
#include "game.h"

void entity_player_init(entity_t * e, vec3_t p1, vec3_t p2);
void entity_player_update(entity_t * e);
void entity_player_receive_damage(entity_t * e, entity_t * from, int32_t amount);
void entity_player_kill(entity_t * e);

void entity_player_constructor(entity_t *e, vec3_t pos, vec3_t p1, vec3_t p2) {

    entity_constructor(e, pos, p1, p2);

    // todo, these casts kinda suck
    e->_init = (void (*)(void *, vec3_t, vec3_t))entity_player_init;
    e->_update = (void (*)(void *))entity_player_update;
    e->_receive_damage = (void (*)(void * e, void * from, int32_t amount))entity_player_receive_damage;
    e->_kill = (void (*)(void *))entity_player_kill;

    // todo, kinda goofy paradigm to set the callback, immediately invoke
    // then never call again. could just combine constructor and init I think
    e->_init(e, p1, p2);
}

void entity_player_init(entity_t * e, vec3_t p1, vec3_t p2) {

    e->s = vec3(12,24,12);
    e->f = 10;
    e->_speed = 3000;
    e->_step_height = 17;
    e->_can_jump = false;
    e->_can_shoot_at = 0.0f;
    e->_health = 100;

    e->_check_against = ENTITY_GROUP_ENEMY;

    // todo, e->_weapons = [new weapon_shotgun_t];
    e->_weapons = NULL;
    e->_weapon_index = 0;

    // Map 1 needs some rotation of the starting look-at direction
    e->_yaw += game_map_index * PI;
    e->_bob = 0;

    game_entity_player = e;

    game_entities_friendly.length++;
    game_entities_friendly.entities = realloc(game_entities_friendly.entities, sizeof(entity_t *) * game_entities_friendly.length);
    game_entities_friendly.entities[game_entities_friendly.length - 1] = e;
}

void entity_player_update(entity_t * e) {}
void entity_player_receive_damage(entity_t * e, entity_t * from, int32_t amount) {}
void entity_player_kill(entity_t * e) {}