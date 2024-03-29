
#include "entity.h"
#include "entity_door.h"
#include "game.h"
#include "text.h"
#include "render.h"

void entity_door_init(entity_t * e, uint8_t dir);
void entity_door_update(entity_t * e);
void entity_door_receive_damage(entity_t * e, entity_t * from, int32_t amount);

void entity_door_constructor(entity_t *e) {

    entity_constructor(e);

    e->_update = entity_door_update;
    e->_receive_damage = entity_door_receive_damage;

    // todo, also set needs_key via params
    char * str_p1 = entity_param_lookup("dir", e->_params->entity_generic_params.extras);
    uint8_t dir = 0;
    if (str_p1)
        dir = atoi(str_p1);

    entity_door_init(e, dir);
}

void entity_door_init(entity_t * e,uint8_t dir) {
    e->_health = 10;
    e->s = vec3(64, 64, 64);
    e->_start_pos = vec3_clone(e->p);

    e->_reset_state_at = 0;
    e->_yaw = (float)dir * PI/2.0f;
    e->_open = 0;
    // Doors block enemies and players
    e->_group = ENTITY_GROUP_ALL;

    // Map 1 only has one door and it needs a key. Should be a flag
    // in the entity data instead :/
    e->_needs_key = (game_map_index == 1);

    entity_set_model(e);
}

float door_text_time = 0.0f;

void entity_door_update(entity_t * e) {
    e->_draw_model(e);

    // check to make sure player is alive and game isn't over
    if ( game_entity_player != NULL && vec3_dist(e->p, game_entity_player->p) < 128 && door_text_time < game_time) {
        if (e->_needs_key) {
            // door text debounce
            door_text_time = game_time + 2.0f;
            // timed_surfaces free at the end of their timer
            text_surface_t * need_key = text_create_surface((font_input_t) {
                .text = "-- door is locked --",
                .color = { .r = 200, .g = 200, .b = 200, .a = 200 },
                .size = FONT_MD
            });
            need_key->x = INTERNAL_W / 2 - need_key->w / 2;
            need_key->y = need_key->h;

            text_push_timed_surface((timed_surface_t) {
                .ts = need_key,
                .ms = 2000,
            });
            return;
        }
        e->_reset_state_at = game_time + 3;
    }

    if (e->_reset_state_at < game_time)
        e->_open = max(0.0f, e->_open-game_tick);
    else
        e->_open = min(1.0f, e->_open + game_tick);

    e->p = vec3_add(e->_start_pos, vec3_rotate_y(vec3(96 * e->_open,0,0), e->_yaw));
}

void entity_door_receive_damage(entity_t * e, entity_t * from, int32_t amount) {
    e = e;
    from = from;
    amount = amount;
}