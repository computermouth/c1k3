
#ifndef ENTITY_ENEMY_H
#define ENTITY_ENEMY_H

#include "entity.h"

void entity_enemy_constructor(entity_t *e, vec3_t pos, uint8_t p1, uint8_t p2);
void entity_enemy_receive_damage(entity_t * e, entity_t * from, int32_t amount);

#endif
