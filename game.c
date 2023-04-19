
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "entity.h"
#include "game.h"
#include "map.h"
#include "math.h"
#include "render.h"
#include "input.h"


state_t game_state = MENU_STATE;
float game_tick = 0.0f;
float game_time = 0.016f;
float game_real_time_last = 0;
float game_message_timeout = 0;

entity_ref_collection_t game_entities = {0};
entity_ref_collection_t game_entities_friendly = {0};
entity_ref_collection_t game_entities_enemies = {0};
entity_t * game_entity_player = NULL;
int game_map_index = 0;
bool game_jump_to_next_level = false;

void game_entities_push(entity_ref_collection_t * c, entity_t * e) {
    c->length++;
    c->entities = realloc(c->entities, sizeof(entity_t *) * c->length);
    c->entities[c->length - 1] = e;
}

void game_entities_pop(entity_ref_collection_t * c, entity_t * e) {
    for(uint32_t i = 0; i < c->length; i++) {
        if (c->entities[i] == e) {
            c->length--;
            // check if it's empty
            if (c->length == 0) {
                free(c->entities);
                c->entities = NULL;
            } else { // otherwise remove
                entity_t * end = c->entities[c->length];
                c->entities[i] = end;
                c->entities = realloc(c->entities, sizeof(entity_t *) * c->length);
            }
        }
    }
}

void game_entities_enemies_push(entity_t * e) {
    game_entities_push(&game_entities_enemies, e);
}

void game_entities_enemies_pop(entity_t * e) {
    game_entities_pop(&game_entities_enemies, e);
}

void game_entities_friendly_push(entity_t * e) {
    game_entities_push(&game_entities_friendly, e);
}

void game_entities_friendly_pop(entity_t * e) {
    game_entities_pop(&game_entities_friendly, e);
}

void game_free_entities() {
    if (game_entities.entities) {
        for(int i = 0; i < game_entities.length; i++) {
            if (game_entities.entities[i]) {
                free(game_entities.entities[i]);
            }
        }
        free(game_entities.entities);
    }
    if (game_entities_friendly.entities)     free(game_entities_friendly.entities);
    if (game_entities_enemies.entities)      free(game_entities_enemies.entities);
    game_entities = (entity_ref_collection_t) {
        0
    };
    game_entities_friendly = (entity_ref_collection_t) {
        0
    };
    game_entities_enemies = (entity_ref_collection_t) {
        0
    };
}

void game_init(int map_index) {

    // cleanup
    game_free_entities();

    game_map_index = map_index;
    map_init(&(map_data.maps[game_map_index]));
}

void game_next_level() {
    game_jump_to_next_level = true;
}

// todo, is this fuckin goofy?
// it is, the pointer that gets returned here is potentially destroyed after update
// functions are ran and game_entities is completely replaced with new values in new
// memory from the new alive_entities collection
entity_t * game_spawn (void (*func)(entity_t *, vec3_t, uint8_t, uint8_t), vec3_t pos, uint8_t p1, uint8_t p2) {
    game_entities.length++;
    game_entities.entities = realloc(game_entities.entities, sizeof(entity_t*) * game_entities.length);
    entity_t * e = calloc(1, sizeof(entity_t));
    game_entities.entities[game_entities.length - 1] = e;

    func(e, pos, p1, p2);
    return e;
}

void game_show_message(char *txt) {
    // todo, show message for period of time
    // clearTimeout(game_message_timeout);
    // game_message_timeout = setTimeout(()=>msg.style.display = 'none', 2000);
    fprintf(stderr, "%s\n", txt);
}

void title_show_message(char *txt, char *sub) {
    // todo, probably change game_show_message to accept size params
    fprintf(stderr, "%s\n", txt);
    fprintf(stderr, "  -- %s\n", sub);
}

void game_run(float time_now) {

    time_now *= 0.001f;
    // todo, sospechoso, not sure what this was for
    // game_tick = min((time_now - (game_real_time_last||time_now)), 0.05f);
    game_tick = (time_now - (game_real_time_last));
    game_real_time_last = time_now;
    game_time += game_tick;

    r_prepare_frame(0.1, 0.2, 0.5);

    if (game_entities_friendly.entities != NULL) {
        free(game_entities_friendly.entities);
        game_entities_friendly = (entity_ref_collection_t) {
            0
        };
    }
    if (game_entities_enemies.entities != NULL) {
        free(game_entities_enemies.entities);
        game_entities_enemies = (entity_ref_collection_t) {
            0
        };
    }

    entity_ref_collection_t alive_entities = {0};
    for (uint32_t i = 0; i < game_entities.length; i++) {
        entity_t * e = game_entities.entities[i];
        if (e->_dead) {
            free(e);
            if (e == game_entity_player) {
                game_entity_player = NULL;
            }
        } else {
            alive_entities.length++;
            alive_entities.entities = realloc(alive_entities.entities, alive_entities.length * sizeof(entity_t) );
            alive_entities.entities[alive_entities.length - 1] = e;
            switch (e->_group) {
            case ENTITY_GROUP_ENEMY:
                game_entities_enemies_push(e);
                break;
            case ENTITY_GROUP_PLAYER:
                game_entities_friendly_push(e);
                break;
            case ENTITY_GROUP_ALL:
                game_entities_enemies_push(e);
                game_entities_friendly_push(e);
                break;
            default:
                break;
            }
        }
    }
    if (game_entities.entities != NULL)
        free(game_entities.entities);
    game_entities = alive_entities;

    for (uint32_t i = 0; i < game_entities.length; i++) {
        entity_t * e = game_entities.entities[i];
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
            title_show_message("THE END", "THANKS FOR PLAYING ❤");
            game_entity_player->_dead = 1;

            // Set camera position for end screen
            r_camera = vec3(1856,784,2272);
            r_camera_yaw = 0;
            r_camera_pitch = 0.5;
        } else {
            game_init(game_map_index);
        }
    }
}