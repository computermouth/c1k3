#ifndef _DATA_
#define _DATA_

#include <stdint.h>

typedef struct {
    uint8_t * data;
    uint64_t len;
} png_bin_t;

extern const png_bin_t * data_textures;
extern const uint32_t data_textures_len;

extern const uint8_t * data_maps;
extern const uint32_t data_maps_len;

extern const uint8_t * data_models;
extern const uint32_t data_models_len;

// == FONTS =====

extern const uint8_t * data_terminess_font;
extern const uint32_t data_terminess_font_len;

// == AUDIO =====

extern const uint8_t * data_sfx_enemy_gib;
extern const uint32_t data_sfx_enemy_gib_len;

extern const uint8_t * data_sfx_enemy_hit;
extern const uint32_t data_sfx_enemy_hit_len;

extern const uint8_t * data_sfx_enemy_hound_attack;
extern const uint32_t data_sfx_enemy_hound_attack_len;

extern const uint8_t * data_sfx_grenade_bounce;
extern const uint32_t data_sfx_grenade_bounce_len;

extern const uint8_t * data_sfx_grenade_explode;
extern const uint32_t data_sfx_grenade_explode_len;

extern const uint8_t * data_sfx_grenade_shoot;
extern const uint32_t data_sfx_grenade_shoot_len;

extern const uint8_t * data_sfx_hurt;
extern const uint32_t data_sfx_hurt_len;

extern const uint8_t * data_sfx_nailgun_hit;
extern const uint32_t data_sfx_nailgun_hit_len;

extern const uint8_t * data_sfx_nailgun_shoot;
extern const uint32_t data_sfx_nailgun_shoot_len;

extern const uint8_t * data_sfx_no_ammo;
extern const uint32_t data_sfx_no_ammo_len;

extern const uint8_t * data_sfx_pickup;
extern const uint32_t data_sfx_pickup_len;

extern const uint8_t * data_sfx_plasma_shoot;
extern const uint32_t data_sfx_plasma_shoot_len;

extern const uint8_t * data_sfx_shotgun_reload;
extern const uint32_t data_sfx_shotgun_reload_len;

extern const uint8_t * data_sfx_shotgun_shoot;
extern const uint32_t data_sfx_shotgun_shoot_len;

extern const uint8_t * data_song;
extern const uint32_t data_song_len;

#endif
