
#include <SDL2/SDL_timer.h>
#include <stdlib.h>

#include "entity.h"
#include "game.h"
#include "map.h"
#include "model.h"
#include "weapon.h"
#include "entity_projectile_shell.h"
#include "entity_projectile_nail.h"
#include "entity_projectile_grenade.h"
#include "audio.h"

void weapon_init(weapon_t * w);
void weapon_shoot(weapon_t * w, vec3_t pos, float yaw, float pitch);
void weapon_spawn_projectile(weapon_t * w, vec3_t pos, float yaw, float pitch);

void weapon_set_model(weapon_t * w, entity_id_t eid) {

    entity_params_t ep = map_entt_params_from_eid(eid);

    w->_texture = ep.entity_generic_params.ref_entt->tex_id;
    w->_model.frames = vector_begin(ep.entity_generic_params.ref_entt->frames);
    w->_model.nv = ep.entity_generic_params.ref_entt->vert_len;
}

weapon_t weapon_constructor() {
    weapon_t w = {0};
    w._found = true;
    w._needs_ammo = 1;
    w._projectile_offset = vec3(0, 0, 8);
    w._init = (void (*)(void * w))weapon_init;
    w._shoot = (void (*)(void * w, vec3_t pos, float yaw, float pitch))weapon_shoot;
    w._spawn_projectile = (void (*)(void * w, vec3_t pos, float yaw, float pitch))weapon_spawn_projectile;

    weapon_init(&w);
    return w;
}

void weapon_init(weapon_t * w) {
    w = w; /*noop*/
}

void weapon_shoot(weapon_t * w, vec3_t pos, float yaw, float pitch) {
    if (w->_needs_ammo)
        w->_ammo--;

    audio_play(w->_sound);
    w->_spawn_projectile(w, pos, yaw, pitch);
}

void weapon_spawn_projectile(weapon_t * w, vec3_t pos, float yaw, float pitch) {

    entity_params_t ep = map_entt_params_from_eid(w->_projectile_type);
    ep.position = vec3_add(
                      pos,
                      vec3_add(
                          vec3(0, 12, 0),
                          vec3_rotate_yaw_pitch(
                              w->_projectile_offset,
                              yaw, pitch
                          )
                      )
                  );

    entity_t * projectile = game_spawn(&ep);

    projectile->v = vec3_rotate_yaw_pitch(
                        vec3(0, 0, w->_projectile_speed),
                        yaw, pitch
                    );

    projectile->_yaw = yaw - PI / 2.0f;
    projectile->_pitch = -pitch + PI / 2.0f;
    projectile->_check_against = ENTITY_GROUP_ENEMY;

    // Alternate left/right fire for next projectile (nailgun)
    w->_projectile_offset.x *= -1;
}

// SHOTGUN ====================================================================

void weapon_shotgun_init(weapon_t * w);
void weapon_shotgun_spawn_projectile(weapon_t * w, vec3_t pos, float yaw, float pitch);

weapon_t weapon_shotgun_constructor() {
    weapon_t w = weapon_constructor();
    w._init = (void (*)(void * w))weapon_shotgun_init;
    w._spawn_projectile = (void (*)(void * w, vec3_t pos, float yaw, float pitch))weapon_shotgun_spawn_projectile;
    w._init(&w);

    return w;
}

void weapon_shotgun_init(weapon_t * w) {
    w->_sound = sfx_shotgun_shoot;
    w->_needs_ammo = 0;
    w->_reload = 0.9f;
    w->_projectile_type = ENTITY_ID_PROJECTILE_SHELL;
    w->_projectile_speed = 10000;

    weapon_set_model(w, ENTITY_ID_PICKUP_SHOTGUN);
}

void weapon_shotgun_spawn_projectile(weapon_t * w, vec3_t pos, float yaw, float pitch) {
    SDL_AddTimer(200, audio_schedule, sfx_shotgun_reload);
    SDL_AddTimer(350, audio_schedule, sfx_shotgun_reload);
    for (uint32_t i = 0; i < 8; i++) {
        weapon_spawn_projectile(w, pos, yaw + randf() * 0.08 - 0.04, pitch + randf() * 0.08 - 0.04);
    }
}

// NAILGUN ====================================================================

void weapon_nailgun_init(weapon_t * w);

weapon_t weapon_nailgun_constructor() {
    weapon_t w = weapon_constructor();
    w._init = (void (*)(void * w))weapon_nailgun_init;
    w._init(&w);

    return w;
}

void weapon_nailgun_init(weapon_t * w) {
    w->_sound = sfx_nailgun_shoot;
    w->_ammo = 100;
    w->_reload = 0.09;
    w->_projectile_type = ENTITY_ID_PROJECTILE_NAIL;
    w->_projectile_speed = 1300;
    w->_projectile_offset = vec3(6,0,8);

    weapon_set_model(w, ENTITY_ID_PICKUP_NAILGUN);
}

// GRENADE LAUNCHER ====================================================================

void weapon_grenadelauncher_init(weapon_t * w);

weapon_t weapon_grenadelauncher_constructor() {
    weapon_t w = weapon_constructor();
    w._init = (void (*)(void * w))weapon_grenadelauncher_init;
    w._init(&w);

    return w;
}

void weapon_grenadelauncher_init(weapon_t * w) {
    w->_sound = sfx_grenade_shoot;
    w->_ammo = 10;
    w->_reload = 0.650;
    w->_projectile_type = ENTITY_ID_PROJECTILE_GRENADE;
    w->_projectile_speed = 900;

    weapon_set_model(w, ENTITY_ID_PICKUP_GRENADELAUNCHER);
}
