
#include "entity_particle.h"
#include "model.h"
#include <sys/types.h>

void entity_particle_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_particle_update(entity_t * e);

// todo, put the models on the stack,
// copy data in
model_t empty = { 0 };

void entity_particle_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_constructor(e, pos, p1, p2);

    // todo, these casts kinda suck
    e->_init = entity_particle_init;
    e->_update = entity_particle_update;

    // todo, kinda goofy paradigm to set the callback, immediately invoke
    // then never call again. could just combine constructor and init I think
    e->_init(e, p1, p2);
    
    // todo, remove once all the _ng's are implemented
    if ( e->_params != NULL && e->_params->entity_generic_params.ref_entt != NULL ){
        e->_texture = e->_params->entity_generic_params.ref_entt->tex_id;
        vector * frames = e->_params->entity_generic_params.ref_entt->frames;
        uint32_t * uframes = vector_begin(frames);
        e->_model = &empty;
        e->_model->frames = uframes;
        e->_model->nv = e->_params->entity_generic_params.ref_entt->vert_len;
        e->s = e->_params->entity_generic_params.ref_entt->size;
    }
    
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