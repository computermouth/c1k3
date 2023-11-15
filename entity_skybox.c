
#include "entity_skybox.h"
#include "game.h"
#include "math.h"
#include "render.h"

void entity_skybox_init(entity_t * e);
void entity_skybox_draw_model(entity_t * e);
void entity_skybox_update(entity_t * e);

void entity_skybox_constructor(entity_t * e) {
    entity_constructor(e);
    e->_draw_model = entity_skybox_draw_model;
    e->_update = entity_skybox_update;
    entity_skybox_init(e);
}

void entity_skybox_init(entity_t * e) {
    // todo
    entity_set_model(e);
    e->s = (vec3_t){0};
}

void entity_skybox_update(entity_t * e) {
    // track player
    if (game_entity_player != NULL) {
        e->p = game_entity_player->p;
    }

    e->_yaw += 0.001 * game_tick;
    // e->_pitch += 0.001 * game_tick;

    e->_draw_model(e);
}

void entity_skybox_draw_model(entity_t * e) {
    draw_call_t call = {
        .pos = e->p,
        .yaw = e->_yaw,
        .pitch = e->_pitch,
        .texture = e->_texture,
        .f1 = e->_model.frames[0],
        .f2 = e->_model.frames[0],
        .mix = 0.0f,
        .unlit = 1,
        .num_verts = e->_model.nv
    };
    r_draw(call);
}