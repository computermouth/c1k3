#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "audio.h"
#include "data.h"
#include "input.h"
#include "render.h"
#include "map.h"
#include "model.h"
#include "game.h"

void game_load() {
    r_init();

    for(int i = 0; i < data_textures_len; i++)
        r_create_texture(data_textures[i]);

    // Load map & model containers
    map_data = map_load_container();
    model_data = model_load_container();

    // Create models. Many models share the same geometry just with different
    // sizes and textures.
    // 0: generic blob
    // 1: humanoid
    // 2: barrel
    // 3: q logo
    // 4: hound
    // 5: box
    // 6: nailgun
    // 7: torch

    model_q = model_init(model_data.models[3].data, (vec3_t) {
        1.0f,1.0f,1.0f
    });
    model_explosion = model_init(model_data.models[0].data, (vec3_t) {
        0.1,0.1,0.1
    });
    model_blood = model_init(model_data.models[0].data, (vec3_t) {
        0.1,0.2,0.1
    });
    model_gib = model_init(model_data.models[0].data, (vec3_t) {
        0.3,0.6,0.3
    });

    model_grunt = model_init(model_data.models[1].data, (vec3_t) {
        2.5,2.2,2.5
    });
    model_enforcer = model_init(model_data.models[1].data, (vec3_t) {
        3,2.7,3
    });
    model_zombie = model_init(model_data.models[1].data, (vec3_t) {
        1.5,2,1.5
    });
    model_ogre = model_init(model_data.models[1].data, (vec3_t) {
        4,3,4
    });
    model_hound = model_init(model_data.models[4].data, (vec3_t) {
        2.5,2.5,2.5
    });

    model_barrel = model_init(model_data.models[2].data, (vec3_t) {
        2, 2, 2
    });
    model_torch = model_init(model_data.models[7].data, (vec3_t) {
        0.6,1,0.6
    });

    model_pickup_nailgun = model_init(model_data.models[6].data, (vec3_t) {
        1, 1, 1
    });
    model_pickup_grenadelauncher = model_init(model_data.models[2].data, (vec3_t) {
        1, 0.5, 0.5
    });
    model_pickup_box = model_init(model_data.models[5].data, (vec3_t) {
        0.7, 0.7, 0.7
    });
    model_pickup_grenades = model_init(model_data.models[5].data, (vec3_t) {
        0.5, 1, 0.5
    });
    model_pickup_key = model_init(model_data.models[5].data, (vec3_t) {
        0.1, 0.7, 0.1
    });

    model_door = model_init(model_data.models[5].data, (vec3_t) {
        5, 5, 0.5
    });

    model_shotgun = model_init(model_data.models[2].data, (vec3_t) {
        1,0.2,0.2
    });
    model_grenadelauncher = model_init(model_data.models[2].data, (vec3_t) {
        0.7,0.4,0.4
    });
    model_nailgun = model_init(model_data.models[6].data, (vec3_t) {
        0.7,0.7,0.7
    });

    model_grenade = model_init(model_data.models[2].data, (vec3_t) {
        0.3,0.3,0.3
    });

    model_nail = model_init(model_data.models[2].data, (vec3_t) {
        0.5,0.1,0.1
    });

    // Take some parts from the grunt model and build individual giblet models
    // from it. Arms and legs and stuff...
    for (uint32_t i = 0; i < 204; i+=34) {
        model_t m = model_init(model_data.models[1].data, vec3(2,1,2));
        m.frames[0] += i;
        m.nv = 34;
        model_gib_pieces.len++;
        model_gib_pieces.models = realloc(model_gib_pieces.models, sizeof(model_t) * model_gib_pieces.len);
        model_gib_pieces.models[model_gib_pieces.len - 1] = m;
    }

    r_submit_buffer();

    audio_init();
};

void menu_run(float time_now) {

    r_prepare_frame(0.0f, 0.0f, 0.0f);

    r_draw((draw_call_t) {
        .pos = vec3(0,0,0),
        .yaw = 0,
        .pitch = 0,
        .texture = 1,
        .f1 = model_q.frames[0],
        .f2 = model_q.frames[0],
        .mix = 0,
        .num_verts = model_q.nv
    });

    r_push_light(
        // +/-200
        vec3(sinf(time_now*0.00033)*200, 100, -100),
        10, 255,192,32
    );
    r_push_light(
        vec3_rotate_y(vec3(0, 0, 100),time_now*0.00063),
        10, 32,64,255
    );
    r_push_light(
        vec3_rotate_y(vec3(100, 0, 0),time_now*0.00053),
        10, 196,128,255
    );

    r_end_frame();

};

void quit() {
    game_free_entities();
    audio_free();
    if (r_draw_calls)
        free(r_draw_calls);
    if (r_textures)
        free(r_textures);
    if (map_data.maps) {
        for(int i = 0; i < map_data.len; i++) {
            if (map_data.maps[i].r) {
                free(map_data.maps[i].r);
            }
        }
        free(map_data.maps);
    }
    if (model_data.models)
        free(model_data.models);
    if (model_q.frames)                      free(model_q.frames);
    if (model_explosion.frames)              free(model_explosion.frames);
    if (model_blood.frames)                  free(model_blood.frames);
    if (model_gib.frames)                    free(model_gib.frames);
    if (model_grunt.frames)                  free(model_grunt.frames);
    if (model_enforcer.frames)               free(model_enforcer.frames);
    if (model_zombie.frames)                 free(model_zombie.frames);
    if (model_ogre.frames)                   free(model_ogre.frames);
    if (model_hound.frames)                  free(model_hound.frames);
    if (model_barrel.frames)                 free(model_barrel.frames);
    if (model_torch.frames)                  free(model_torch.frames);
    if (model_shotgun.frames)                free(model_shotgun.frames);
    if (model_nailgun.frames)                free(model_nailgun.frames);
    if (model_grenadelauncher.frames)        free(model_grenadelauncher.frames);
    if (model_pickup_nailgun.frames)         free(model_pickup_nailgun.frames);
    if (model_pickup_grenadelauncher.frames) free(model_pickup_grenadelauncher.frames);
    if (model_pickup_box.frames)             free(model_pickup_box.frames);
    if (model_pickup_grenades.frames)        free(model_pickup_grenades.frames);
    if (model_pickup_key.frames)             free(model_pickup_key.frames);
    if (model_door.frames)                   free(model_door.frames);
    if (model_grenade.frames)                free(model_grenade.frames);
    if (model_nail.frames)                   free(model_nail.frames);
}

int main() {

    // todo, more randy
    time_t t;
    srand((unsigned) time(&t));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    // if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
    //     printf( "SDL_mixer could not initialize! SDL_Error: %s\n", SDL_GetError() );
    // }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512 ) != 0) {
        printf( "SDL_mixer could not initialize! SDL_Error: %s\n", Mix_GetError() );
    }
    // Requires at least OpenGL ES 2.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
    SDL_Window* window = SDL_CreateWindow("c1k3",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          D_WINDOW_W, D_WINDOW_H,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_GL_CreateContext(window);
    // SDL_GL_SetSwapInterval(0);
    // todo, vsync?
    //   SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    //   SDL_Texture * target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
    // SDL_TEXTUREACCESS_TARGET, 320, 180);
    //   SDL_RenderClear(renderer);
    //   SDL_SetRenderTarget(renderer, target);

    game_load();

    time(&t);
    int oldtime = t;
    int newtime = t;
    int frames = 0;

    while (1) {
        frames++;
        time(&t);
        newtime = t;
        if (newtime - oldtime >=2) {
            printf("fps: %f\n", (float)frames / 2.0f);
            oldtime = newtime;
            frames = 0;
        }

        input_consume();

        if (input_quit)
            goto jump;

        if(game_state == MENU_STATE && keys[KEY_ACTION] ) {
            game_state = GAME_STATE;
            Mix_PlayMusic(song, -1);
            game_init(0);
            mouse_x = 0.0f;
            mouse_y = 0.0f;
            keys[KEY_NEXT] = 0;
            keys[KEY_PREV] = 0;
            // don't shoot on first frame
            keys[KEY_ACTION] = 0;
        }

        // perform based on state
        // move event handling into update loop
        // todo, move
        if (game_state == GAME_STATE)
            game_run(SDL_GetTicks()); // time in ms since page load?
        else
            menu_run(SDL_GetTicks());
        // SDL_SetRenderTarget(renderer, NULL);
        // SDL_RenderCopy(renderer, target, NULL, NULL);
        // SDL_RenderPresent(renderer);
        // SDL_SetRenderTarget(renderer, target);

        SDL_GL_SwapWindow(window);

    }

    const char * serror = SDL_GetError();
    while ( strcmp(serror, "") ) {
        printf("sdlerror: %s\n", serror);
        serror = SDL_GetError();
    }

    const char * merror = Mix_GetError();
    while ( strcmp(merror, "") ) {
        printf("mixerror: %s\n", merror);
        serror = SDL_GetError();
    }

    GLenum gerror = glGetError();
    while (gerror != GL_NO_ERROR) {
        printf("glerror: %x\n", gerror);
        gerror = glGetError();
    }

jump:

    quit();
}
