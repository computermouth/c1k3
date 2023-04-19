#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "data.h"
#include "input.h"
#include "render.h"
#include "map.h"
#include "model.h"
#include "game.h"

// Sounds
void * sfx_enemy_hit = NULL;
void * sfx_enemy_gib = NULL;
void * sfx_enemy_hound_attack = NULL;
void * sfx_no_ammo = NULL;
void * sfx_hurt = NULL;
void * sfx_pickup = NULL;
void * sfx_plasma_shoot = NULL;
void * sfx_shotgun_shoot = NULL;
void * sfx_shotgun_reload = NULL;
void * sfx_nailgun_shoot = NULL;
void * sfx_nailgun_hit = NULL;
void * sfx_grenade_shoot = NULL;
void * sfx_grenade_bounce = NULL;
void * sfx_grenade_explode = NULL;

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
    /*

    f.onclick = () => g.requestFullscreen();
    g.onclick = () => {
    	g.onclick = () => c.requestPointerLock();
    	g.onclick();

    	audio_init();

    	// Generate sounds
    	sfx_enemy_hit = audio_create_sound(135, [8,0,0,1,148,1,3,5,0,0,139,1,0,2653,0,2193,255,2,639,119,2,23,0,0,0,0,0,0,0]);
    	sfx_enemy_gib = audio_create_sound(140, [7,0,0,1,148,1,7,5,0,1,139,1,0,4611,789,15986,195,2,849,119,3,60,0,0,0,1,10,176,1]);
    	sfx_enemy_hound_attack = audio_create_sound(132, [8,0,0,1,192,1,8,0,0,1,120,1,0,5614,0,20400,192,1,329,252,1,55,0,0,1,1,8,192,3]);

    	sfx_no_ammo = audio_create_sound(120, [8,0,0,0,96,1,8,0,0,0,0,0,255,0,0,1075,232,1,2132,255,0,0,0,0,0,0,0,0,0]);
    	sfx_hurt = audio_create_sound(135, [7,3,140,1,232,3,8,0,9,1,139,3,0,4611,1403,34215,256,4,1316,255,0,0,0,1,0,1,7,255,0]);
    	sfx_pickup = audio_create_sound(140, [7,0,0,1,187,3,8,0,0,1,204,3,0,4298,927,1403,255,0,0,0,3,35,0,0,0,0,0,0,0]);

    	sfx_plasma_shoot = audio_create_sound(135, [8,0,0,1,147,1,6,0,0,1,159,1,0,197,1234,21759,232,2,2902,255,2,53,0,0,0,0,0,0,0]);

    	sfx_shotgun_shoot = audio_create_sound(135, [7,3,0,1,255,1,6,0,0,1,255,1,112,548,1979,11601,255,2,2902,176,2,77,0,0,1,0,10,255,1]);
    	sfx_shotgun_reload = audio_create_sound(125, [9,0,0,1,131,1,0,0,0,0,0,3,255,137,22,1776,255,2,4498,176,2,36,2,84,0,0,3,96,0]);

    	sfx_nailgun_shoot = audio_create_sound(130, [7,0,0,1,132,1,8,4,0,1,132,2,162,0,0,8339,232,2,2844,195,2,40,0,0,0,0,0,0,0]);
    	sfx_nailgun_hit = audio_create_sound(135, [8,0,0,1,148,1,0,0,0,0,0,1,255,0,0,2193,128,2,6982,119,2,23,0,0,0,0,0,0,0]);

    	sfx_grenade_shoot = audio_create_sound(127, [8,0,0,1,171,1,9,3,0,1,84,3,96,2653,0,13163,159,2,3206,255,2,64,0,0,0,1,9,226,0]);
    	sfx_grenade_bounce = audio_create_sound(168, [7,0,124,0,128,0,8,5,127,0,128,0,125,88,0,2193,125,1,1238,240,1,91,3,47,0,0,0,0,0]);
    	sfx_grenade_explode = audio_create_sound(135, [8,0,0,1,195,1,6,0,0,1,127,1,255,197,1234,21759,232,2,1052,255,4,73,3,25,1,0,10,227,1]);


    	audio_play(audio_create_song(...music_data), 1, 1);
    	game_init(0);
        // set state to render menu
    	run_frame = game_run;

    */
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
    if (game_entities.entities) {
        for(int i = 0; i < game_entities.length; i++) {
            if (game_entities.entities[i]) {
                free(game_entities.entities[i]);
            }
        }
        free(game_entities.entities);
    }
    if (game_entities_friendly.entities)     free(game_entities_friendly.entities);
    if (game_entities_enemies.entities)      free(game_entities_enemies.entities);
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

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    // Requires at least OpenGL ES 2.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
    SDL_Window* window = SDL_CreateWindow("c1k3",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          // 320, 180,
                                          960, 540,
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

    GLenum gerror = glGetError();
    while (gerror != GL_NO_ERROR) {
        printf("glerror: %x\n", gerror);
        gerror = glGetError();
    }

jump:

    quit();
}
