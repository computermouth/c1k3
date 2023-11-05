
#include <SDL2/SDL.h>
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
#include "vector.h"

state_t game_state = MENU_STATE;
float game_tick = 0.0f;
float game_time = 0.016f;
float game_message_timeout = 0;

// todo, can __game_entities be remade to be contiguous memory?
// maybe lists are indices into game_entities?
vector * __game_entities = NULL;
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

    if (__game_entities) {
        size_t ge_len = vector_size(__game_entities);
        for(size_t i = 0; i < ge_len; i++) {
            entity_t ** e = vector_at(__game_entities, i);
            if(*e)
                free(*e);
        }
    }

    vector_free(__game_entities);
    __game_entities = NULL;

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
    __game_entities = vector_init(sizeof(entity_t *));

    // entity_t *
    game_entities_list_all = vector_init(sizeof(entity_t *));
    game_entities_list_friendly = vector_init(sizeof(entity_t *));
    game_entities_list_enemies = vector_init(sizeof(entity_t *));

    game_map_index = map_index;
    map_set_level(game_map_index);
    map_load();

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
    // silence unused warnings
    interval = interval;
    param = param;

    game_map_index = 0;
    game_reset_level = 1;
    return 0;
}

// todo, fix this import
typedef void (*constfunc)(entity_t *);
extern constfunc map_constfunc_from_eid(entity_id_t eid);

entity_t * game_spawn (entity_params_t * ep) {

    void (*constructor)(entity_t *) = map_constfunc_from_eid(ep->id);
    if(ep->id >= __ENTITY_ID_END) {
        fprintf(stderr, "E: unimp -- %d\n", ep->id);
        return NULL;
    }

    entity_t * e = calloc(1, sizeof(entity_t));
    e->_params = ep;
    vector_push(__game_entities, &e);
    constructor(e);

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
    uint32_t len = vector_size(__game_entities);
    for (uint32_t i = 0; i < len; i++) {
        entity_t ** pe = vector_at(__game_entities, i);
        entity_t * e = *pe;
        if (e->_dead) {
            if (e == game_entity_player)
                game_entity_player = NULL;
            free(e);
            vector_erase(__game_entities, i);
            i--;
            len--;
        } else {
            switch (e->_group) {
            case ENTITY_GROUP_ENEMY:
                game_entities_all_push(&e);
                game_entities_enemies_push(&e);
                break;
            case ENTITY_GROUP_PLAYER:
                game_entities_all_push(&e);
                game_entities_friendly_push(&e);
                break;
            case ENTITY_GROUP_ALL:
                game_entities_all_push(&e);
                game_entities_enemies_push(&e);
                game_entities_friendly_push(&e);
                break;
            default:
                game_entities_all_push(&e);
                break;
            }
        }
    }

    // run updates after _dead entities have been removed
    len = vector_size(__game_entities);
    for (uint32_t i = 0; i < len; i++) {
        entity_t ** te = vector_at(__game_entities, i);
        entity_t * e = *te;
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
