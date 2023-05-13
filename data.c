
#include "data.h"

#include "sfx_enemy_gib.h"
#include "sfx_enemy_hit.h"
#include "sfx_enemy_hound_attack.h"
#include "sfx_grenade_bounce.h"
#include "sfx_grenade_explode.h"
#include "sfx_grenade_shoot.h"
#include "sfx_hurt.h"
#include "sfx_nailgun_hit.h"
#include "sfx_nailgun_shoot.h"
#include "sfx_no_ammo.h"
#include "sfx_pickup.h"
#include "sfx_plasma_shoot.h"
#include "sfx_shotgun_reload.h"
#include "sfx_shotgun_shoot.h"
#include "song.h"
#include "TerminessNerdFontMono-Bold.h"

const uint8_t * data_terminess_font = ttf_TerminessNerdFontMono_Bold_ttf;
const uint32_t data_terminess_font_len = ttf_TerminessNerdFontMono_Bold_ttf_len;

const uint8_t * data_sfx_enemy_gib = audio_sfx_enemy_gib_ogg;
const uint32_t data_sfx_enemy_gib_len = audio_sfx_enemy_gib_ogg_len;

const uint8_t * data_sfx_enemy_hit = audio_sfx_enemy_hit_ogg;
const uint32_t data_sfx_enemy_hit_len = audio_sfx_enemy_hit_ogg_len;

const uint8_t * data_sfx_enemy_hound_attack = audio_sfx_enemy_hound_attack_ogg;
const uint32_t data_sfx_enemy_hound_attack_len = audio_sfx_enemy_hound_attack_ogg_len;

const uint8_t * data_sfx_grenade_bounce = audio_sfx_grenade_bounce_ogg;
const uint32_t data_sfx_grenade_bounce_len = audio_sfx_grenade_bounce_ogg_len;

const uint8_t * data_sfx_grenade_explode = audio_sfx_grenade_explode_ogg;
const uint32_t data_sfx_grenade_explode_len = audio_sfx_grenade_explode_ogg_len;

const uint8_t * data_sfx_grenade_shoot = audio_sfx_grenade_shoot_ogg;
const uint32_t data_sfx_grenade_shoot_len = audio_sfx_grenade_shoot_ogg_len;

const uint8_t * data_sfx_hurt = audio_sfx_hurt_ogg;
const uint32_t data_sfx_hurt_len = audio_sfx_hurt_ogg_len;

const uint8_t * data_sfx_nailgun_hit = audio_sfx_nailgun_hit_ogg;
const uint32_t data_sfx_nailgun_hit_len = audio_sfx_nailgun_hit_ogg_len;

const uint8_t * data_sfx_nailgun_shoot = audio_sfx_nailgun_shoot_ogg;
const uint32_t data_sfx_nailgun_shoot_len = audio_sfx_nailgun_shoot_ogg_len;

const uint8_t * data_sfx_no_ammo = audio_sfx_no_ammo_ogg;
const uint32_t data_sfx_no_ammo_len = audio_sfx_no_ammo_ogg_len;

const uint8_t * data_sfx_pickup = audio_sfx_pickup_ogg;
const uint32_t data_sfx_pickup_len = audio_sfx_pickup_ogg_len;

const uint8_t * data_sfx_plasma_shoot = audio_sfx_plasma_shoot_ogg;
const uint32_t data_sfx_plasma_shoot_len = audio_sfx_plasma_shoot_ogg_len;

const uint8_t * data_sfx_shotgun_reload = audio_sfx_shotgun_reload_ogg;
const uint32_t data_sfx_shotgun_reload_len = audio_sfx_shotgun_reload_ogg_len;

const uint8_t * data_sfx_shotgun_shoot = audio_sfx_shotgun_shoot_ogg;
const uint32_t data_sfx_shotgun_shoot_len = audio_sfx_shotgun_shoot_ogg_len;

// const uint8_t * data_song = file_example_OOG_1MG_ogg;
// const uint32_t data_song_len = file_example_OOG_1MG_ogg_len;

const uint8_t * data_song = audio_song_ogg;
const uint32_t data_song_len = audio_song_ogg_len;

#include "maps.h"
// #include "assets/testmap/testmap.h"

const uint8_t * data_maps = maps;
const uint32_t data_maps_len = maps_len;

#include "models.h"

const uint8_t * data_models = models;
const uint32_t data_models_len = models_len;

#include  "0.h"
#include  "1.h"
#include  "2.h"
#include  "3.h"
#include  "4.h"
#include  "5.h"
#include  "6.h"
#include  "7.h"
#include  "8.h"
#include  "9.h"
#include "10.h"
#include "11.h"
#include "12.h"
#include "13.h"
#include "14.h"
#include "15.h"
#include "16.h"
#include "17.h"
#include "18.h"
#include "19.h"
#include "20.h"
#include "21.h"
#include "22.h"
#include "23.h"
#include "24.h"
#include "25.h"
#include "26.h"
#include "27.h"
#include "28.h"
#include "29.h"
#include "30.h"

#define TEXLEN 31

const png_bin_t __alltex[TEXLEN] = {
    {
        .data = img_0_png,
        .len  = img_0_png_len,
    },
    {
        .data = img_1_png,
        .len  = img_1_png_len,
    },
    {
        .data = img_2_png,
        .len  = img_2_png_len,
    },
    {
        .data = img_3_png,
        .len  = img_3_png_len,
    },
    {
        .data = img_4_png,
        .len  = img_4_png_len,
    },
    {
        .data = img_5_png,
        .len  = img_5_png_len,
    },
    {
        .data = img_6_png,
        .len  = img_6_png_len,
    },
    {
        .data = img_7_png,
        .len  = img_7_png_len,
    },
    {
        .data = img_8_png,
        .len  = img_8_png_len,
    },
    {
        .data = img_9_png,
        .len  = img_9_png_len,
    },
    {
        .data = img_10_png,
        .len  = img_10_png_len,
    },
    {
        .data = img_11_png,
        .len  = img_11_png_len,
    },
    {
        .data = img_12_png,
        .len  = img_12_png_len,
    },
    {
        .data = img_13_png,
        .len  = img_13_png_len,
    },
    {
        .data = img_14_png,
        .len  = img_14_png_len,
    },
    {
        .data = img_15_png,
        .len  = img_15_png_len,
    },
    {
        .data = img_16_png,
        .len  = img_16_png_len,
    },
    {
        .data = img_17_png,
        .len  = img_17_png_len,
    },
    {
        .data = img_18_png,
        .len  = img_18_png_len,
    },
    {
        .data = img_19_png,
        .len  = img_19_png_len,
    },
    {
        .data = img_20_png,
        .len  = img_20_png_len,
    },
    {
        .data = img_21_png,
        .len  = img_21_png_len,
    },
    {
        .data = img_22_png,
        .len  = img_22_png_len,
    },
    {
        .data = img_23_png,
        .len  = img_23_png_len,
    },
    {
        .data = img_24_png,
        .len  = img_24_png_len,
    },
    {
        .data = img_25_png,
        .len  = img_25_png_len,
    },
    {
        .data = img_26_png,
        .len  = img_26_png_len,
    },
    {
        .data = img_27_png,
        .len  = img_27_png_len,
    },
    {
        .data = img_28_png,
        .len  = img_28_png_len,
    },
    {
        .data = img_29_png,
        .len  = img_29_png_len,
    },
    {
        .data = img_30_png,
        .len  = img_30_png_len,
    }
};

const png_bin_t * data_textures = __alltex;
const uint32_t data_textures_len = TEXLEN;
