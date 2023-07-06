
#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "entity.h"
#include "game.h"
#include "map.h"
#include "math.h"
#include "render.h"
#include "input.h"
#include "text.h"

state_t game_state = MENU_STATE;
float game_tick = 0.0f;
float game_time = 0.016f;
float game_message_timeout = 0;

// entity_t
vector * game_entities = NULL;

// entity_t *
vector * game_entities_list_all = NULL;
vector * game_entities_list_friendly = NULL;
vector * game_entities_list_enemies = NULL;

entity_t * game_entity_player = NULL;
int game_map_index = 0;
int game_reset_level = 0;
bool game_jump_to_next_level = false;

void game_entities_push(vector * v, entity_t ** e) {
    vector_push(v, e);
}

void game_entities_pop(vector * v, entity_t ** e) {
    uint32_t len = vector_size(v);
    for(uint32_t i = 0; i < len; i++) {
        if (vector_at(v, i) == e) {
            vector_erase(v, i);
        }
    }
}

void game_entities_all_push(entity_t ** e) {
    game_entities_push(game_entities_list_all, e);
}

void game_entities_all_pop(entity_t ** e) {
    game_entities_pop(game_entities_list_all, e);
}

void game_entities_enemies_push(entity_t ** e) {
    game_entities_push(game_entities_list_enemies, e);
}

void game_entities_enemies_pop(entity_t ** e) {
    game_entities_pop(game_entities_list_enemies, e);
}

void game_entities_friendly_push(entity_t ** e) {
    game_entities_push(game_entities_list_friendly, e);
}

void game_entities_friendly_pop(entity_t ** e) {
    game_entities_pop(game_entities_list_friendly, e);
}

void game_free_entities() {

    vector_free(game_entities);
    game_entities = NULL;

    vector_free(game_entities_list_all);
    vector_free(game_entities_list_friendly);
    vector_free(game_entities_list_enemies);

    game_entities_list_all = NULL;
    game_entities_list_friendly = NULL;
    game_entities_list_enemies = NULL;

}

void game_init(int map_index) {

    // cleanup
    game_free_entities();

    // entity_t
    game_entities = vector_init(sizeof(entity_t));

    // entity_t *
    game_entities_list_all = vector_init(sizeof(entity_t *));
    game_entities_list_friendly = vector_init(sizeof(entity_t *));
    game_entities_list_enemies = vector_init(sizeof(entity_t *));

    game_map_index = map_index;
    map_t * map = vector_at(map_data, game_map_index);
    map_load(map);

    // make sure that first frame won't have
    // massive ticks, and also not divide by 0's
    // just fake one 60Hz frame
    float time_now = SDL_GetTicks();
    time_now *= 0.001f;
    game_tick = 0.016f;
    game_time = (time_now - game_tick);
}

void game_next_level() {
    game_jump_to_next_level = true;
}

uint32_t game_finish(uint32_t interval, void *param) {
    game_map_index = 0;
    game_reset_level = 1;
    return 0;
}

// todo, is this fuckin goofy?
// it is, the pointer that gets returned here is potentially destroyed after update
// functions are ran and game_entities is completely replaced with new values in new
// memory from the new alive_entities collection
entity_t * game_spawn (void (*init)(entity_t *, vec3_t, uint8_t, uint8_t), vec3_t pos, uint8_t p1, uint8_t p2) {

    entity_t * e = vector_push(game_entities, &(entity_t) {
        0
    });
    init(e, pos, p1, p2);

    return e;
}

void game_run(float time_now) {

    time_now *= 0.001f;
    game_tick = (time_now - game_time);
    if (game_tick <= 0.001f)
        game_tick = 0.001f;
    game_time = time_now;

    r_prepare_frame(0.1, 0.2, 0.5);

    vector_clear(game_entities_list_all);
    vector_clear(game_entities_list_enemies);
    vector_clear(game_entities_list_friendly);

    // todo, should be some kind of timed callback on death
    // dead, restart level -- also maybe remove null check
    if (game_reset_level) {
        game_reset_level = 0;
        game_init(game_map_index);
    }

    // clean up _dead entities
    uint32_t len = vector_size(game_entities);
    for (uint32_t i = 0; i < len; i++) {
        entity_t * e = vector_at(game_entities, i);
        if (e->_dead) {
            if (e == game_entity_player)
                game_entity_player = NULL;
            vector_erase(game_entities, i);
            i--;
            len--;
        } else {
            switch (e->_group) {
            case ENTITY_GROUP_ENEMY:
                game_entities_enemies_push(&e);
                break;
            case ENTITY_GROUP_PLAYER:
                game_entities_friendly_push(&e);
                break;
            case ENTITY_GROUP_ALL:
                game_entities_all_push(&e);
                game_entities_enemies_push(&e);
                game_entities_friendly_push(&e);
                break;
            default:
                break;
            }
        }
    }

    // run updates after _dead entities have been removed
    len = vector_size(game_entities);
    for (uint32_t i = 0; i < len; i++) {
        entity_t * e = vector_at(game_entities, i);
        e->_update(e);
    }

    map_draw();
    r_end_frame();

    // reset mouse movement and buttons that can't be held
    mouse_x = 0.0f;
    mouse_y = 0.0f;
    keys[KEY_NEXT] = 0;
    keys[KEY_PREV] = 0;

    if (game_jump_to_next_level) {
        game_jump_to_next_level = 0;
        game_map_index++;
        if (game_map_index == 2) {

            // timed_surfaces free at the end of their timer
            text_surface_t * end_text = text_create_surface(
            (font_input_t) {
                .text = "THE END",
                .color = { .r = 255, .g = 255, .b = 255, .a = 255 },
                .size = FONT_LG
            });
            end_text->x = INTERNAL_W / 2 - end_text->w / 2;
            end_text->y = end_text->h;

            text_surface_t * thx_text = text_create_surface(
            (font_input_t) {
                .text = "-- thanks for playing <3 --",
                .color = { .r = 200, .g = 200, .b = 200, .a = 200 },
                .size = FONT_SM
            });
            thx_text->x = INTERNAL_W / 2 - thx_text->w / 2;
            thx_text->y = end_text->y + end_text->h + thx_text->h;

            text_push_timed_surface((timed_surface_t) {
                .ts = end_text,
                .ms = 5000,
            });

            text_push_timed_surface((timed_surface_t) {
                .ts = thx_text,
                .ms = 5000,
            });

            game_entity_player->_dead = 1;

            // Set camera position for end screen
            r_camera = vec3(1856,784,2272);
            r_camera_yaw = 0;
            r_camera_pitch = 0.5;

            SDL_AddTimer(5000, game_finish, NULL);
        } else {
            game_init(game_map_index);
        }
    }
}
