
#include "entity_pickup_key.h"
#include "entity_pickup.h"
#include "game.h"
#include "audio.h"
#include "text.h"
#include "render.h"

void entity_pickup_key_init(entity_t * e);
void entity_pickup_key_update(entity_t * e);
void entity_pickup_key_pickup(entity_t * e);

void entity_pickup_key_constructor(entity_t * e) {
    entity_pickup_constructor(e);
    e->_update = entity_pickup_key_update;
    e->_pickup = entity_pickup_key_pickup;
    entity_pickup_key_init(e);
}

void entity_pickup_key_init(entity_t * e) {
    entity_set_model(e);
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