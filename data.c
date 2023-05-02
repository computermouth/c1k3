
#include "data.h"

#include "assets/audio.h/sfx_enemy_gib.h"
#include "assets/audio.h/sfx_enemy_hit.h"
#include "assets/audio.h/sfx_enemy_hound_attack.h"
#include "assets/audio.h/sfx_grenade_bounce.h"
#include "assets/audio.h/sfx_grenade_explode.h"
#include "assets/audio.h/sfx_grenade_shoot.h"
#include "assets/audio.h/sfx_hurt.h"
#include "assets/audio.h/sfx_nailgun_hit.h"
#include "assets/audio.h/sfx_nailgun_shoot.h"
#include "assets/audio.h/sfx_no_ammo.h"
#include "assets/audio.h/sfx_pickup.h"
#include "assets/audio.h/sfx_plasma_shoot.h"
#include "assets/audio.h/sfx_shotgun_reload.h"
#include "assets/audio.h/sfx_shotgun_shoot.h"
// #include "assets/audio.wav/song.h"
#include "assets/audio.h/song.h"
// #include "assets/audio.h/example.h"

const uint8_t * data_sfx_enemy_gib = sfx_enemy_gib_ogg;
const uint32_t data_sfx_enemy_gib_len = sfx_enemy_gib_ogg_len;

const uint8_t * data_sfx_enemy_hit = sfx_enemy_hit_ogg;
const uint32_t data_sfx_enemy_hit_len = sfx_enemy_hit_ogg_len;

const uint8_t * data_sfx_enemy_hound_attack = sfx_enemy_hound_attack_ogg;
const uint32_t data_sfx_enemy_hound_attack_len = sfx_enemy_hound_attack_ogg_len;

const uint8_t * data_sfx_grenade_bounce = sfx_grenade_bounce_ogg;
const uint32_t data_sfx_grenade_bounce_len = sfx_grenade_bounce_ogg_len;

const uint8_t * data_sfx_grenade_explode = sfx_grenade_explode_ogg;
const uint32_t data_sfx_grenade_explode_len = sfx_grenade_explode_ogg_len;

const uint8_t * data_sfx_grenade_shoot = sfx_grenade_shoot_ogg;
const uint32_t data_sfx_grenade_shoot_len = sfx_grenade_shoot_ogg_len;

const uint8_t * data_sfx_hurt = sfx_hurt_ogg;
const uint32_t data_sfx_hurt_len = sfx_hurt_ogg_len;

const uint8_t * data_sfx_nailgun_hit = sfx_nailgun_hit_ogg;
const uint32_t data_sfx_nailgun_hit_len = sfx_nailgun_hit_ogg_len;

const uint8_t * data_sfx_nailgun_shoot = sfx_nailgun_shoot_ogg;
const uint32_t data_sfx_nailgun_shoot_len = sfx_nailgun_shoot_ogg_len;

const uint8_t * data_sfx_no_ammo = sfx_no_ammo_ogg;
const uint32_t data_sfx_no_ammo_len = sfx_no_ammo_ogg_len;

const uint8_t * data_sfx_pickup = sfx_pickup_ogg;
const uint32_t data_sfx_pickup_len = sfx_pickup_ogg_len;

const uint8_t * data_sfx_plasma_shoot = sfx_plasma_shoot_ogg;
const uint32_t data_sfx_plasma_shoot_len = sfx_plasma_shoot_ogg_len;

const uint8_t * data_sfx_shotgun_reload = sfx_shotgun_reload_ogg;
const uint32_t data_sfx_shotgun_reload_len = sfx_shotgun_reload_ogg_len;

const uint8_t * data_sfx_shotgun_shoot = sfx_shotgun_shoot_ogg;
const uint32_t data_sfx_shotgun_shoot_len = sfx_shotgun_shoot_ogg_len;

// const uint8_t * data_song = file_example_OOG_1MG_ogg;
// const uint32_t data_song_len = file_example_OOG_1MG_ogg_len;

const uint8_t * data_song = song_ogg;
const uint32_t data_song_len = song_ogg_len;

#include "assets/maps.h"
// #include "assets/testmap/testmap.h"

const uint8_t * data_maps = __maps;
const uint32_t data_maps_len = __maps_len;

#include "assets/models.h"

const uint8_t * data_models = __models;
const uint32_t data_models_len = __models_len;

#include  "assets/img.h/0.h"
#include  "assets/img.h/1.h"
#include  "assets/img.h/2.h"
#include  "assets/img.h/3.h"
#include  "assets/img.h/4.h"
#include  "assets/img.h/5.h"
#include  "assets/img.h/6.h"
#include  "assets/img.h/7.h"
#include  "assets/img.h/8.h"
#include  "assets/img.h/9.h"
#include "assets/img.h/10.h"
#include "assets/img.h/11.h"
#include "assets/img.h/12.h"
#include "assets/img.h/13.h"
#include "assets/img.h/14.h"
#include "assets/img.h/15.h"
#include "assets/img.h/16.h"
#include "assets/img.h/17.h"
#include "assets/img.h/18.h"
#include "assets/img.h/19.h"
#include "assets/img.h/20.h"
#include "assets/img.h/21.h"
#include "assets/img.h/22.h"
#include "assets/img.h/23.h"
#include "assets/img.h/24.h"
#include "assets/img.h/25.h"
#include "assets/img.h/26.h"
#include "assets/img.h/27.h"
#include "assets/img.h/28.h"
#include "assets/img.h/29.h"
#include "assets/img.h/30.h"

#define TEXLEN 31

const png_bin_t __alltex[TEXLEN] = {
    {
        .data = __0_png,
        .len  = __0_png_len,
    },
    {
        .data = __1_png,
        .len  = __1_png_len,
    },
    {
        .data = __2_png,
        .len  = __2_png_len,
    },
    {
        .data = __3_png,
        .len  = __3_png_len,
    },
    {
        .data = __4_png,
        .len  = __4_png_len,
    },
    {
        .data = __5_png,
        .len  = __5_png_len,
    },
    {
        .data = __6_png,
        .len  = __6_png_len,
    },
    {
        .data = __7_png,
        .len  = __7_png_len,
    },
    {
        .data = __8_png,
        .len  = __8_png_len,
    },
    {
        .data = __9_png,
        .len  = __9_png_len,
    },
    {
        .data = __10_png,
        .len  = __10_png_len,
    },
    {
        .data = __11_png,
        .len  = __11_png_len,
    },
    {
        .data = __12_png,
        .len  = __12_png_len,
    },
    {
        .data = __13_png,
        .len  = __13_png_len,
    },
    {
        .data = __14_png,
        .len  = __14_png_len,
    },
    {
        .data = __15_png,
        .len  = __15_png_len,
    },
    {
        .data = __16_png,
        .len  = __16_png_len,
    },
    {
        .data = __17_png,
        .len  = __17_png_len,
    },
    {
        .data = __18_png,
        .len  = __18_png_len,
    },
    {
        .data = __19_png,
        .len  = __19_png_len,
    },
    {
        .data = __20_png,
        .len  = __20_png_len,
    },
    {
        .data = __21_png,
        .len  = __21_png_len,
    },
    {
        .data = __22_png,
        .len  = __22_png_len,
    },
    {
        .data = __23_png,
        .len  = __23_png_len,
    },
    {
        .data = __24_png,
        .len  = __24_png_len,
    },
    {
        .data = __25_png,
        .len  = __25_png_len,
    },
    {
        .data = __26_png,
        .len  = __26_png_len,
    },
    {
        .data = __27_png,
        .len  = __27_png_len,
    },
    {
        .data = __28_png,
        .len  = __28_png_len,
    },
    {
        .data = __29_png,
        .len  = __29_png_len,
    },
    {
        .data = __30_png,
        .len  = __30_png_len,
    }
};

const png_bin_t * data_textures = __alltex;
const uint32_t data_textures_len = TEXLEN;
