
#include "entity_pickup_key.h"
#include "entity_pickup.h"
#include "game.h"
#include "audio.h"
#include "text.h"
#include "render.h"

void entity_pickup_key_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_pickup_key_update(entity_t * e);
void entity_pickup_key_pickup(entity_t * e);

void entity_pickup_key_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_pickup_constructor(e, pos, p1, p2);
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_pickup_key_init;
    e->_update = (void (*)(void *))entity_pickup_key_update;
    e->_pickup = (void (*)(void *))entity_pickup_key_pickup;
    e->_init(e, p1, p2);
}

void entity_pickup_key_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_texture = 21;
    e->_model = &model_pickup_key;
}

void entity_pickup_key_update(entity_t * e) {
    e->_yaw += 0.02;
    entity_pickup_update(e);
}

void entity_pickup_key_pickup(entity_t * e) {
    audio_play(sfx_pickup);

    // timed_surfaces free at the end of their timer
    // timed_surfaces free at the end of their timer
    text_surface_t * found_key = text_create_surface((font_input_t) {
        .text = "-- key acquired --",
        .color = { .r = 200, .g = 200, .b = 200, .a = 200 },
        .size = FONT_MD
    });
    found_key->x = INTERNAL_W / 2 - found_key->w / 2;
    found_key->y = found_key->h;

    found_key->x = INTERNAL_W / 2 - found_key->w / 2;
    found_key->y = found_key->h;

    text_push_timed_surface((timed_surface_t) {
        .ts = found_key,
        .ms = 2000,
    });

    uint32_t len = vector_size(game_entities_list_all);
    for (uint32_t i = 0; i < len; i++) {
        entity_t ** door_p = vector_at(game_entities_list_all, i);
        entity_t * door = *door_p;
        if (door->_needs_key) {
            door->_needs_key = 0;
            break;
        }
    }
    e->_kill(e);
}