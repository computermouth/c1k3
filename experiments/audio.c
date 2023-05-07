#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

int main(int argc, char** argv) {
    // Initialize SDL2 mixer
    int flags = MIX_INIT_OGG;
    int initted = Mix_Init(flags);
    if ((initted & flags) != flags) {
        printf("Failed to initialize SDL2 mixer: %s\n", Mix_GetError());
        return 1;
    }

    // Open the audio device
    int channels = 2;
    int chunksize = 2048;
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, channels, chunksize) < 0) {
        printf("Failed to open audio device: %s\n", Mix_GetError());
        return 1;
    }
    
    Mix_AllocateChannels(32);

    // Load the OGG files
    Mix_Chunk* effects[14];
	effects[0] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[1] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[2] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[3] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[4] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[5] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[6] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[7] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[8] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[9] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[10] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[11] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[12] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");
	effects[13] = Mix_LoadWAV("sm2_sfx_enemy_gib.ogg");

    // Play the sound effects
    for (int i = 0; i < 14; i++) {
        int channel = Mix_PlayChannel(-1, effects[i], 0);
        if (channel == -1) {
            printf("[%d] Failed to play effect: %s\n", i, Mix_GetError());
        }
    }

    // Wait for the sound effects to finish
    SDL_Delay(5000); // wait for 5 seconds

    // Free the loaded files
    for (int i = 0; i < 10; i++) {
        Mix_FreeChunk(effects[i]);
    }

    // Close the audio device
    Mix_CloseAudio();

    // Quit SDL2 mixer
    Mix_Quit();

    return 0;
}
