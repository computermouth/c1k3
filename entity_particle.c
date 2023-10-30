
#include "entity_particle.h"
#include "model.h"
#include <sys/types.h>

void entity_particle_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_particle_update(entity_t * e);

void entity_particle_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_constructor(e, pos, p1, p2);

    e->_update = entity_particle_update;
    entity_particle_init(e, p1, p2);
}

void entity_particle_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_bounciness = 0.5f;
    e->f = 0.1f;
    
    entity_set_model(e);
}

void entity_particle_update(entity_t * e) {
    e->_yaw += e->v.y * 0.001;
    e->_pitch += e->v.x * 0.001;
    e->_update_physics(e);
    e->_draw_model(e);
}