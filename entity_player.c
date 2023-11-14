
#include <SDL2/SDL.h>

#include <stdbool.h>
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "entity.h"
#include "entity_light.h"
#include "entity_player.h"
#include "game.h"
#include "map.h"
#include "math.h"
#include "input.h"
#include "render.h"
#include "game.h"
#include "text.h"
#include "weapon.h"
#include "input.h"
#include "audio.h"

void entity_player_init(entity_t * e);
void entity_player_update(entity_t * e);
void entity_player_receive_damage(entity_t * e, entity_t * from, int32_t amount);
void entity_player_kill(entity_t * e);

void entity_player_constructor(entity_t * e) {

    entity_constructor(e);

    // todo, parameter for spawn face direction
    //
    // Map 1 needs some rotation of the starting look-at direction
    // e->_yaw += game_map_index * PI;

    e->_update = entity_player_update;
    e->_receive_damage = entity_player_receive_damage;
    e->_kill = entity_player_kill;

    entity_player_init(e);
}

typedef struct {
    int32_t health;
    int32_t ammo;
    char hlth_text[20];
    char ammo_text[20];
    text_surface_t * hlth_surface;
    text_surface_t * ammo_surface;
} hud_t;

hud_t hud = { 0 };

void entity_player_hud_update_health(int32_t new_health) {
    hud.health = new_health;
    char * h = "\U0000F004";
    snprintf(hud.hlth_text, 20, "%s: %3d", h, new_health);

    if (hud.hlth_surface)
        text_free_surface(hud.hlth_surface);

    hud.hlth_surface = text_create_surface((font_input_t) {
        .text = hud.hlth_text,
        .color = { .r = 255, .g = 255, .b = 255, .a = 255 },
        .size = FONT_MD
    });
    hud.hlth_surface->x = (INTERNAL_W / 6) - hud.hlth_surface->w / 2;
    hud.hlth_surface->y = INTERNAL_H  - (hud.hlth_surface->h * 2);
}

void entity_player_hud_update_ammo(int32_t new_ammo) {
    hud.ammo = new_ammo;
    char * b = "\U000F0572";
    if (new_ammo == -1)
        snprintf(hud.ammo_text, 20, "%s:  ∞ ", b);
    else
        snprintf(hud.ammo_text, 20, "%s: %3d", b, new_ammo);

    if (hud.ammo_surface)
        text_free_surface(hud.ammo_surface);

    hud.ammo_surface = text_create_surface((font_input_t) {
        .text = hud.ammo_text,
        .color = { .r = 255, .g = 255, .b = 255, .a = 255 },
        .size = FONT_MD
    });
    hud.ammo_surface->x = INTERNAL_W - hud.hlth_surface->x - hud.ammo_surface->w;
    hud.ammo_surface->y = INTERNAL_H  - (hud.ammo_surface->h * 2);
}

void entity_player_init(entity_t * e) {

    e->s = vec3(12,24,12);
    e->f = 10;
    e->_speed = 3000;
    e->_step_height = 17;
    e->_can_jump = false;
    e->_can_shoot_at = 0.0f;
    e->_health = 100;

    e->_group = ENTITY_GROUP_PLAYER;
    e->_check_against = ENTITY_GROUP_ENEMY;

    // todo, e->_weapons = [new weapon_shotgun_t];
    e->_weapons[0] = weapon_shotgun_constructor();
    e->_weapons[1] = (weapon_t) {
        0
    };
    e->_weapons[2] = (weapon_t) {
        0
    };
    e->_weapon_index = 0;

    e->_bob = 0;

    game_entity_player = e;

    // setup hud
    hud.health = e->_health;
    hud.ammo = -1;

    // spawn skybox
    entity_params_t skybox_ep = map_entt_params_from_eid(ENTITY_ID_SKYBOX);
    skybox_ep.position = e->p;
    game_spawn(&skybox_ep);

    entity_player_hud_update_health(hud.health);
    entity_player_hud_update_ammo(hud.ammo);
}

void entity_player_update(entity_t * e) {
    // Mouse look
    e->_pitch = clamp(
                    e->_pitch + mouse_y * mouse_speed * (mouse_invert ? -0.00015 : 0.00015),
                    -1.5f, 1.5f
                );
    e->_yaw = fmodf((e->_yaw + mouse_x * mouse_speed * 0.00015), PI * 2);

    // Acceleration in movement direction
    e->a = vec3_mulf(
               vec3_rotate_y(
                   vec3(
                       keys[KEY_RIGHT] - keys[KEY_LEFT],
                       0,
                       keys[KEY_UP] - keys[KEY_DOWN]
                   ),
                   e->_yaw
               ),
               e->_speed * (e->_on_ground ? 1 : 0.3)
           );

    if (keys[KEY_JUMP] && e->_on_ground && e->_can_jump) {
        e->v.y = 400;
        e->_on_ground = 0;
        e->_can_jump = 0;
    }
    if (!keys[KEY_JUMP]) {
        e->_can_jump = 1;
    }

    if (keys[KEY_NEXT]) {
        do {
            e->_weapon_index = (e->_weapon_index + 1) % WEAPON_END;
        } while (e->_weapons[e->_weapon_index]._found == false);
    }

    if (keys[KEY_PREV]) {
        do {
            e->_weapon_index = (WEAPON_END + e->_weapon_index - 1) % WEAPON_END;
        } while (e->_weapons[e->_weapon_index]._found == false);
    }

    float shoot_wait = e->_can_shoot_at - game_time;
    weapon_t * weapon = &(e->_weapons[e->_weapon_index]);

    // Shoot Weapon
    if (keys[KEY_ACTION] && shoot_wait < 0) {
        e->_can_shoot_at = game_time + weapon->_reload;

        if (weapon->_needs_ammo && weapon->_ammo == 0) {
            audio_play(sfx_no_ammo);
        }
        else {
            weapon->_shoot(weapon, e->p, e->_yaw, e->_pitch);

            entity_params_t l = {
                .id = ENTITY_ID_LIGHT,
                .position = e->p,
                .entity_light_params = {
                    .rgba[0] = 0xff,
                    .rgba[1] = 0xff,
                    .rgba[2] = 0xff,
                    .rgba[3] = 0x0a,
                },
            };
            entity_t * tmp_light = game_spawn(&l);
            tmp_light->_expires = game_time + 0.1;
            tmp_light->_die_at = game_time + 0.1;
        }
    }

    e->_bob += vec3_length(e->a) * game_tick * 0.01;
    e->f = e->_on_ground ? 10 : 2.5;
    e->_update_physics(e);

    r_camera.x = e->p.x;
    r_camera.z = e->p.z;

    // Smooth step up on stairs
    r_camera.y = e->p.y + 8 - clamp(game_time - e->_stepped_up_at, 0, 0.1) * -160;

    r_camera_yaw = e->_yaw;
    r_camera_pitch = e->_pitch;

    // Draw weapon at camera position at an offset and add the current
    // recoil (calculated from shoot_wait and weapon._reload) accounting
    // for the current view yaw/pitch

    draw_call_t d = {
        .pos = vec3_add(
            r_camera,
            vec3_rotate_yaw_pitch(
                vec3(
                    0,
                    -10 + sinf(e->_bob)*0.3,
                    12 + clamp(scale(shoot_wait, 0, weapon->_reload, 5, 0), 0, 5)
                ),
                e->_yaw, e->_pitch
            )
        ),
        .yaw = e->_yaw + PI / 2.0f,
        .pitch = e->_pitch,
        .texture = weapon->_texture,
        .f1 = weapon->_model.frames[0],
        .f2 = weapon->_model.frames[0],
        .mix = 0,
        .num_verts = weapon->_model.nv
    };

    r_draw(d);

    int32_t tmp_health = e->_health;
    int32_t tmp_ammo = weapon->_ammo;
    if (! weapon->_needs_ammo)
        weapon->_ammo = -1;

    if (hud.health != tmp_health)
        entity_player_hud_update_health(tmp_health);

    if (hud.ammo != tmp_ammo)
        entity_player_hud_update_ammo(tmp_ammo);

    text_push_surface(hud.hlth_surface);
    text_push_surface(hud.ammo_surface);

}

void entity_player_receive_damage(entity_t * e, entity_t * from, int32_t amount) {
    audio_play(sfx_hurt);
    // invoke regular damage thing
    entity_receive_damage(e, from, amount);
}

uint32_t entity_player_reset_level(uint32_t interval, void *param) {
    // silence unused
    interval = interval;
    param = param;

    game_reset_level = 1;
    return 0;
}

void entity_player_kill(entity_t * e) {
    entity_kill(e);

    // timed_surfaces free at the end of their timer
    text_surface_t * died_text = text_create_surface(
    (font_input_t) {
        .text = "YOU DIED",
        .color = { .r = 200, .g = 50, .b = 50, .a = 200 },
        .size = FONT_LG
    });

    died_text->x = INTERNAL_W / 2 - died_text->w / 2;
    died_text->y = died_text->h;

    text_push_timed_surface((timed_surface_t) {
        .ts = died_text,
        .ms = 2000,
    });

    text_push_surface(died_text);

    if (hud.hlth_surface)
        text_free_surface(hud.hlth_surface);
    if (hud.ammo_surface)
        text_free_surface(hud.ammo_surface);

    hud = (hud_t) {
        0
    };

    SDL_AddTimer(2000, entity_player_reset_level, e);
}