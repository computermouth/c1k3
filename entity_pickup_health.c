
#include "entity_pickup_health.h"
#include "entity_pickup.h"
#include "game.h"
#include "audio.h"

void entity_pickup_health_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_pickup_health_pickup(entity_t * e);

void entity_pickup_health_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_pickup_constructor(e, pos, p1, p2);
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_pickup_health_init;
    e->_pickup = (void (*)(void *))entity_pickup_health_pickup;
    e->_init(e, p1, p2);

    e->_texture = e->_params->entity_generic_params.ref_entt->tex_id;
    vector * frames = e->_params->entity_generic_params.ref_entt->frames;
    uint32_t * uframes = vector_begin(frames);
    e->_model->frames = uframes;
    e->_model->nv = e->_params->entity_generic_params.ref_entt->vert_len;
}

void entity_pickup_health_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_texture = 23;
}

void entity_pickup_health_pickup(entity_t * e) {
    game_entity_player->_health += 25;
    audio_play(sfx_pickup);
    e->_kill(e);
}