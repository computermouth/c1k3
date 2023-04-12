
#include "entity_particle.h"

void entity_particle_init(entity_t * e, vec3_t p1, vec3_t p2);
void entity_particle_update(entity_t * e);

entity_t entity_particle_constructor(vec3_t pos, vec3_t p1, vec3_t p2) {
    entity_t tmp = entity_constructor(pos, p1, p2);

    // todo, these casts kinda suck
    tmp._init = (void (*)(void *, vec3_t, vec3_t))entity_particle_init;
    tmp._update = (void (*)(void *))entity_particle_update;

    // todo, kinda goofy paradigm to set the callback, immediately invoke
    // then never call again. could just combine constructor and init I think
    tmp._init(&tmp, p1, p2);
    return tmp;
}

void entity_particle_init(entity_t * e, vec3_t p1, vec3_t p2) {
    e->_bounciness = 0.5f;
    e->f = 0.1f;
}

void entity_particle_update(entity_t * e) {
    e->_yaw += e->v.y * 0.001;
    e->_pitch += e->v.x * 0.001;
    e->_update_physics(e);
    e->_draw_model(e);
}