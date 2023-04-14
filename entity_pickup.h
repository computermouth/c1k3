
#ifndef _ENTITY_PICKUP_
#define _ENTITY_PICKUP_

#include "entity.h"

void entity_pickup_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2);
void entity_pickup_update(entity_t * e);

#endif