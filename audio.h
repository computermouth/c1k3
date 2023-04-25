
#ifndef _AUDIO_
#define _AUDIO_

#include <SDL2/SDL_mixer.h>

extern Mix_Chunk * sfx_enemy_hit;
extern Mix_Chunk * sfx_enemy_gib;
extern Mix_Chunk * sfx_enemy_hound_attack;
extern Mix_Chunk * sfx_no_ammo;
extern Mix_Chunk * sfx_hurt;
extern Mix_Chunk * sfx_pickup;
extern Mix_Chunk * sfx_plasma_shoot;
extern Mix_Chunk * sfx_shotgun_shoot;
extern Mix_Chunk * sfx_shotgun_reload;
extern Mix_Chunk * sfx_nailgun_shoot;
extern Mix_Chunk * sfx_nailgun_hit;
extern Mix_Chunk * sfx_grenade_shoot;
extern Mix_Chunk * sfx_grenade_bounce;
extern Mix_Chunk * sfx_grenade_explode;

extern Mix_Music * song;

void audio_init();
void audio_play(Mix_Chunk * c);
void audio_play_opt(Mix_Chunk * c, float volume, int32_t loops, float pan);
uint32_t audio_schedule(uint32_t interval, void *param);
void audio_free();

#endif