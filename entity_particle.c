
#include "entity_particle.h"

void entity_particle_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_particle_update(entity_t * e);

void entity_particle_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2, entity_params_t * ep) {
    entity_constructor(e, pos, p1, p2, ep);

    // todo, these casts kinda suck
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_particle_init;
    e->_update = (void (*)(void *))entity_particle_update;

    // todo, kinda goofy paradigm to set the callback, immediately invoke
    // then never call again. could just combine constructor and init I think
    e->_init(e, p1, p2);
}

void entity_particle_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_bounciness = 0.5f;
    e->f = 0.1f;
}

void entity_particle_update(entity_t * e) {
    e->_yaw += e->v.y * 0.001;
    e->_pitch += e->v.x * 0.001;
    e->_update_physics(e);
    e->_draw_model(e);
}