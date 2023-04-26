
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_timer.h>
#include <stdint.h>

#include "audio.h"
#include "data.h"

Mix_Chunk * sfx_enemy_hit = NULL;
Mix_Chunk * sfx_enemy_gib = NULL;
Mix_Chunk * sfx_enemy_hound_attack = NULL;
Mix_Chunk * sfx_no_ammo = NULL;
Mix_Chunk * sfx_hurt = NULL;
Mix_Chunk * sfx_pickup = NULL;
Mix_Chunk * sfx_plasma_shoot = NULL;
Mix_Chunk * sfx_shotgun_shoot = NULL;
Mix_Chunk * sfx_shotgun_reload = NULL;
Mix_Chunk * sfx_nailgun_shoot = NULL;
Mix_Chunk * sfx_nailgun_hit = NULL;
Mix_Chunk * sfx_grenade_shoot = NULL;
Mix_Chunk * sfx_grenade_bounce = NULL;
Mix_Chunk * sfx_grenade_explode = NULL;

Mix_Music * song = NULL;

void audio_init() {
    // todo, loop over, and index into with enum?
    SDL_RWops * rw = NULL;

    rw = SDL_RWFromMem((void *)data_sfx_enemy_hit, data_sfx_enemy_hit_len);
    sfx_enemy_hit = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_enemy_gib, data_sfx_enemy_gib_len);
    sfx_enemy_gib = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_enemy_hound_attack, data_sfx_enemy_hound_attack_len);
    sfx_enemy_hound_attack = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_no_ammo, data_sfx_no_ammo_len);
    sfx_no_ammo = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_hurt, data_sfx_hurt_len);
    sfx_hurt = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_pickup, data_sfx_pickup_len);
    sfx_pickup = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_plasma_shoot, data_sfx_plasma_shoot_len);
    sfx_plasma_shoot = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_shotgun_shoot, data_sfx_shotgun_shoot_len);
    sfx_shotgun_shoot = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_shotgun_reload, data_sfx_shotgun_reload_len);
    sfx_shotgun_reload = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_nailgun_shoot, data_sfx_nailgun_shoot_len);
    sfx_nailgun_shoot = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_nailgun_hit, data_sfx_nailgun_hit_len);
    sfx_nailgun_hit = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_grenade_shoot, data_sfx_grenade_shoot_len);
    sfx_grenade_shoot = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_grenade_bounce, data_sfx_grenade_bounce_len);
    sfx_grenade_bounce = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_sfx_grenade_explode, data_sfx_grenade_explode_len);
    sfx_grenade_explode = Mix_LoadWAV_RW(rw, 1);

    rw = SDL_RWFromMem((void *)data_song, data_song_len);
    song = Mix_LoadMUS_RW(rw, 1);
}

void audio_play_opt(Mix_Chunk * c, float volume, int32_t loops, float pan) {
    // todo, something better with volume
    // test if modification of this volume while playing fucks iwht other samples
    c->volume = volume * 128.0f;
    // uint8_t left = pan * 255.0f;
    // int Mix_SetPanning(int channel, left, 255 - left);
    Mix_PlayChannel(-1, c, loops);
}

void audio_play(Mix_Chunk * c) {
    audio_play_opt(c, 1.0, 0, 0.5);
}

uint32_t audio_schedule(uint32_t interval, void *param) {
    audio_play(param);
    return 0;
}

void audio_free() {
    Mix_FreeChunk(sfx_enemy_hit);
    Mix_FreeChunk(sfx_enemy_gib);
    Mix_FreeChunk(sfx_enemy_hound_attack);
    Mix_FreeChunk(sfx_no_ammo);
    Mix_FreeChunk(sfx_hurt);
    Mix_FreeChunk(sfx_pickup);
    Mix_FreeChunk(sfx_plasma_shoot);
    Mix_FreeChunk(sfx_shotgun_shoot);
    Mix_FreeChunk(sfx_shotgun_reload);
    Mix_FreeChunk(sfx_nailgun_shoot);
    Mix_FreeChunk(sfx_nailgun_hit);
    Mix_FreeChunk(sfx_grenade_shoot);
    Mix_FreeChunk(sfx_grenade_bounce);
    Mix_FreeChunk(sfx_grenade_explode);

    Mix_FreeMusic(song);
    Mix_CloseAudio();
}
