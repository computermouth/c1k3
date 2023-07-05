#include <GLES2/gl2.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "SDL_timer.h"
#include "audio.h"
#include "data.h"
#include "input.h"
#include "render.h"
#include "map.h"
#include "model.h"
#include "game.h"
#include "text.h"
#include "vector.h"

void game_load() {
    // text has to come before render init
    // not sure why, todo
    text_init();
    r_init();

    for(int i = 0; i < data_textures_len; i++)
        r_create_texture(data_textures[i]);

    // Load map & model containers
    map_parse();
    model_parse();

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

    model_t * tmp_mod = NULL;

    tmp_mod = vector_at(model_data, 3);
    model_q = model_load(tmp_mod->data, (vec3_t) {
        1.0f,1.0f,1.0f
    });

    tmp_mod = vector_at(model_data, 0);
    model_explosion = model_load(tmp_mod->data, (vec3_t) {
        0.1,0.1,0.1
    });

    tmp_mod = vector_at(model_data, 0);
    model_blood = model_load(tmp_mod->data, (vec3_t) {
        0.1,0.2,0.1
    });

    tmp_mod = vector_at(model_data, 0);
    model_gib = model_load(tmp_mod->data, (vec3_t) {
        0.3,0.6,0.3
    });

    tmp_mod = vector_at(model_data, 1);
    model_grunt = model_load(tmp_mod->data, (vec3_t) {
        2.5,2.2,2.5
    });

    tmp_mod = vector_at(model_data, 1);
    model_enforcer = model_load(tmp_mod->data, (vec3_t) {
        3,2.7,3
    });

    tmp_mod = vector_at(model_data, 1);
    model_zombie = model_load(tmp_mod->data, (vec3_t) {
        1.5,2,1.5
    });

    tmp_mod = vector_at(model_data, 1);
    model_ogre = model_load(tmp_mod->data, (vec3_t) {
        4,3,4
    });

    tmp_mod = vector_at(model_data, 4);
    model_hound = model_load(tmp_mod->data, (vec3_t) {
        2.5,2.5,2.5
    });

    tmp_mod = vector_at(model_data, 2);
    model_barrel = model_load(tmp_mod->data, (vec3_t) {
        2, 2, 2
    });

    tmp_mod = vector_at(model_data, 7);
    model_torch = model_load(tmp_mod->data, (vec3_t) {
        0.6,1,0.6
    });

    tmp_mod = vector_at(model_data, 6);
    model_pickup_nailgun = model_load(tmp_mod->data, (vec3_t) {
        1, 1, 1
    });

    tmp_mod = vector_at(model_data, 2);
    model_pickup_grenadelauncher = model_load(tmp_mod->data, (vec3_t) {
        1, 0.5, 0.5
    });

    tmp_mod = vector_at(model_data, 5);
    model_pickup_box = model_load(tmp_mod->data, (vec3_t) {
        0.7, 0.7, 0.7
    });

    tmp_mod = vector_at(model_data, 5);
    model_pickup_grenades = model_load(tmp_mod->data, (vec3_t) {
        0.5, 1, 0.5
    });

    tmp_mod = vector_at(model_data, 5);
    model_pickup_key = model_load(tmp_mod->data, (vec3_t) {
        0.1, 0.7, 0.1
    });

    tmp_mod = vector_at(model_data, 5);
    model_door = model_load(tmp_mod->data, (vec3_t) {
        5, 5, 0.5
    });

    tmp_mod = vector_at(model_data, 2);
    model_shotgun = model_load(tmp_mod->data, (vec3_t) {
        1,0.2,0.2
    });

    tmp_mod = vector_at(model_data, 2);
    model_grenadelauncher = model_load(tmp_mod->data, (vec3_t) {
        0.7,0.4,0.4
    });

    tmp_mod = vector_at(model_data, 6);
    model_nailgun = model_load(tmp_mod->data, (vec3_t) {
        0.7,0.7,0.7
    });

    tmp_mod = vector_at(model_data, 2);
    model_grenade = model_load(tmp_mod->data, (vec3_t) {
        0.3,0.3,0.3
    });

    tmp_mod = vector_at(model_data, 2);
    model_nail = model_load(tmp_mod->data, (vec3_t) {
        0.5,0.1,0.1
    });

    // Take some parts from the grunt model and build individual giblet models
    // from it. Arms and legs and stuff...
    tmp_mod = vector_at(model_data, 1);
    for (uint32_t i = 0; i < 204; i+=34) {
        model_t m = model_load(tmp_mod->data, vec3(2,1,2));
        m.frames[0] += i;
        m.nv = 34;
        vector_push(model_gib_pieces, &m);
    }

    r_submit_buffer();

    audio_init();
};

text_surface_t * c1k3 = NULL;
text_surface_t * dq = NULL;

void menu_run(float time_now) {

    if (!c1k3) {
        c1k3 = text_create_surface((font_input_t) {
            .text = "C1K3",
            .color = { .r = 255, .g = 255, .b = 255, .a = 255 },
            .size = FONT_LG
        });
        c1k3->x = INTERNAL_W / 2 - c1k3->w / 2;
        c1k3->y = INTERNAL_H / 2 - c1k3->h / 2;

        dq = text_create_surface((font_input_t) {
            .text = "-- dequake fps --",
            .color = { .r = 200, .g = 200, .b = 200, .a = 200 },
            .size = FONT_SM
        });
        dq->x = INTERNAL_W / 2 - dq->w / 2;
        dq->y = INTERNAL_H / 2 + dq->h;
    }

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

    text_push_surface(c1k3);
    text_push_surface(dq);

    r_end_frame();

};

void quit() {
    game_free_entities();
    audio_free();
    text_quit();
    render_quit();
    model_quit();
    map_quit();

    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {

    // todo, more randy
    time_t t;
    srand((unsigned) time(&t));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
        fprintf(stderr, "SDL_mixer could not Mix_Init! SDL_Error: %s\n", SDL_GetError() );
    }
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048 ) != 0) {
        fprintf(stderr, "SDL_mixer could not Mix_OpenAudio! SDL_Error: %s\n", Mix_GetError() );
        fprintf(stderr, "  -- this could just mean that no audio output exists" );
    }
    // todo, tune?
    Mix_AllocateChannels(32);
    // Requires at least OpenGL ES 2.0
    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
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
            fprintf(stderr, "fps: %f\n", (float)frames / 2.0f);
            oldtime = newtime;
            frames = 0;
        }

        input_consume();

        if (input_quit)
            goto end;

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
            if (c1k3)
                text_free_surface(c1k3);
            if (dq)
                text_free_surface(dq);
        }

        // perform based on state
        // move event handling into update loop
        // todo, move
        if (game_state == GAME_STATE)
            game_run(SDL_GetTicks()); // time in ms since page load?
        else
            menu_run(SDL_GetTicks());

        // todo, move to render?
        SDL_GL_SwapWindow(window);

        const char * serror = NULL;
        while ( strcmp((serror = SDL_GetError()), "") ) {
            fprintf(stderr, "sdlerror: %s\n", serror);
            SDL_ClearError();
            serror = SDL_GetError();
        }

        GLenum gerror = GL_NO_ERROR;
        while ((gerror = glGetError()) != GL_NO_ERROR) {
            fprintf(stderr, "glerror: %x\n", gerror);
        }

    }

end:

    quit();

    return 0;
}
