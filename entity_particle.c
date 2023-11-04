
#include "entity_particle.h"
#include "model.h"
#include "game.h"

void entity_particle_init(entity_t * e);
void entity_particle_update(entity_t * e);

void entity_particle_constructor(entity_t * e) {
    entity_constructor(e);

    e->_update = entity_particle_update;
    entity_particle_init(e);
}

void entity_particle_init(entity_t * e) {
    e->_bounciness = 0.5f;
    e->f = 0.1f;

    entity_set_model(e);
}

void entity_particle_update(entity_t * e) {
    e->_yaw += e->v.y * game_tick * 0.1;
    e->_pitch += e->v.x * game_tick * 0.1;
    e->_update_physics(e);
    e->_draw_model(e);
}