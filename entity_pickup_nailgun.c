
#include "entity_pickup_nailgun.h"
#include "entity.h"
#include "entity_pickup.h"
#include "game.h"
#include "weapon.h"
#include "audio.h"

void entity_pickup_nailgun_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_pickup_nailgun_update(entity_t * e);
void entity_pickup_nailgun_pickup(entity_t * e);
void entity_pickup_nailgun_draw_model(entity_t * e);

void entity_pickup_nailgun_constructor(entity_t *e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_pickup_constructor(e, pos, p1, p2);
    e->_init = entity_pickup_nailgun_init;
    e->_update = entity_pickup_nailgun_update;
    e->_pickup = entity_pickup_nailgun_pickup;
    e->_init(e, p1, p2);

    e->_texture = e->_params->entity_generic_params.ref_entt->tex_id;
    vector * frames = e->_params->entity_generic_params.ref_entt->frames;
    uint32_t * uframes = vector_begin(frames);
    e->_model.frames = uframes;
    e->_model.nv = e->_params->entity_generic_params.ref_entt->vert_len;
}

void entity_pickup_nailgun_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_texture = 12;
    // e->_model = &model_pickup_nailgun;
}

void entity_pickup_nailgun_update(entity_t * e) {
    e->_yaw += 0.02;
    entity_pickup_update(e);
}

void entity_pickup_nailgun_pickup(entity_t * e) {
    audio_play(sfx_pickup);

    game_entity_player->_weapons[WEAPON_NAILGUN] = weapon_nailgun_constructor();
    game_entity_player->_weapon_index = WEAPON_NAILGUN;

    e->_kill(e);
}