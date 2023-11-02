
#ifndef ENTITY_ENEMY_H
#define ENTITY_ENEMY_H

#include "entity.h"

void entity_enemy_constructor(entity_t *e, uint8_t patrol);
void entity_enemy_receive_damage(entity_t * e, entity_t * from, int32_t amount);

#endif
