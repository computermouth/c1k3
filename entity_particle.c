
#include "entity_particle.h"
#include "model.h"
#include <sys/types.h>

void entity_particle_init(entity_t * e);
void entity_particle_update(entity_t * e);

void entity_particle_constructor(entity_t * e, vec3_t pos) {
    entity_constructor(e, pos);

    e->_update = entity_particle_update;
    entity_particle_init(e);
}

void entity_particle_init(entity_t * e) {
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