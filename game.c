
#include <stdint.h>
#include <stdlib.h>

#include "entity.h"
#include "game.h"
#include "map.h"
#include "math.h"
#include "render.h"
#include "input.h"

float game_tick = 0.0f;
float game_time = 0.016f;
float game_real_time_last = 0;
float game_message_timeout = 0;

entity_collection_t game_entities = {0};
entity_collection_t game_entities_friendly = {0};
entity_collection_t game_entities_enemies = {0};
entity_t game_entity_player = {0};
int game_map_index = 0;
bool game_jump_to_next_level = false;

void game_init(int map_index) {

    // probably for reloads
    game_entities = (entity_collection_t) {
        0
    };
    game_entities_friendly = (entity_collection_t) {
        0
    };
    game_entities_enemies = (entity_collection_t) {
        0
    };

    game_map_index = map_index;
    map_init(&(map_data.maps[game_map_index]));
}

void game_next_level() {
    game_jump_to_next_level = true;
}

// todo, is this fuckin goofy?
// pos -> game_spawn -> constructor -> init -> constructor -> init
entity_t game_spawn (entity_t (*func)(), vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_t tmp = func(pos, p1, p2);
    game_entities.length++;
    game_entities.entities = realloc(game_entities.entities, sizeof(entity_t) * game_entities.length);
    game_entities.entities[game_entities.length - 1] = tmp;
    return tmp;
}

void game_show_message(char *txt) {
    // todo, show message for period of time
    // clearTimeout(game_message_timeout);
    // game_message_timeout = setTimeout(()=>msg.style.display = 'none', 2000);
    txt[0] = txt[0];
}

void title_show_message(char *txt, char *sub) {
    // todo, probably change game_show_message to accept size params
    txt[0] = txt[0];
}

void game_run(float time_now) {

    time_now *= 0.001f;
    // todo, sospechoso
    game_tick = min((time_now - (game_real_time_last||time_now)), 0.05f);
    game_real_time_last = time_now;
    game_time += game_tick;

    r_prepare_frame(0.1, 0.2, 0.5);
    entity_collection_t alive_entities = {0};
    for (uint32_t i = 0; i < game_entities.length; i++) {
        entity_t * e = &(game_entities.entities[i]);
        if (!e->_dead) {
            e->_update(e);
            alive_entities.length++;
            alive_entities.entities = realloc(alive_entities.entities, sizeof(entity_t) * alive_entities.length);
            alive_entities.entities[alive_entities.length - 1] = *e;
        }
    }
    if (game_entities.entities != NULL)
        free(game_entities.entities);
    game_entities = alive_entities;

    map_draw();
    r_end_frame();

    mouse_x = 0.0f;
    mouse_y = 0.0f;

    // todo, keymap stuff
    // keys[key_next] = 0;
    // keys[keys_prev] = 0;

    if (game_jump_to_next_level) {
        game_jump_to_next_level = 0;
        game_map_index++;
        if (game_map_index == 2) {
            title_show_message("THE END", "THANKS FOR PLAYING ❤");
            game_entity_player._dead = 1;

            // Set camera position for end screen
            r_camera = vec3(1856,784,2272);
            r_camera_yaw = 0;
            r_camera_pitch = 0.5;
        } else {
            game_init(game_map_index);
        }
    }
}