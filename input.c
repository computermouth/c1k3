
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_video.h>

#include "input.h"
#include "game.h"
#include "render.h"

float mouse_x = 0.0f;
float mouse_y = 0.0f;
float last_wheel_event = 0.0f;
int mouse_speed = 10; // 0 - 50
bool mouse_invert = false;
bool keys[_KEY_END] = {0};
bool input_quit = false;

void input_consume() {

    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
        if(e.type == SDL_QUIT)
            input_quit = true;
        if(e.type == SDL_KEYDOWN && e.key.repeat == 0) {
            switch (e.key.keysym.sym) {
            case SDLK_UP:
            case SDLK_w:
                keys[KEY_UP] = 1;
                break;
            case SDLK_LEFT:
            case SDLK_a:
                keys[KEY_LEFT] = 1;
                break;
            case SDLK_DOWN:
            case SDLK_s:
                keys[KEY_DOWN] = 1;
                break;
            case SDLK_RIGHT:
            case SDLK_d:
                keys[KEY_RIGHT] = 1;
                break;
            case SDLK_q:
                keys[KEY_PREV] = 1;
                break;
            case SDLK_e:
                keys[KEY_NEXT] = 1;
                break;
            case SDLK_SPACE:
                keys[KEY_JUMP] = 1;
                break;
            }
        }
        if(e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
                SDL_SetRelativeMouseMode(false);
                break;
            case SDLK_UP:
            case SDLK_w:
                keys[KEY_UP] = 0;
                break;
            case SDLK_LEFT:
            case SDLK_a:
                keys[KEY_LEFT] = 0;
                break;
            case SDLK_DOWN:
            case SDLK_s:
                keys[KEY_DOWN] = 0;
                break;
            case SDLK_RIGHT:
            case SDLK_d:
                keys[KEY_RIGHT] = 0;
                break;
            case SDLK_q:
                keys[KEY_PREV] = 0;
                break;
            case SDLK_e:
                keys[KEY_NEXT] = 0;
                break;
            case SDLK_SPACE:
                keys[KEY_JUMP] = 0;
                break;
            }
        }
        if(e.type == SDL_MOUSEBUTTONDOWN) {
            switch (e.button.button) {
            case SDL_BUTTON_LEFT:
                if (SDL_GetRelativeMouseMode() == false) {
                    SDL_SetRelativeMouseMode(true);
                    break;
                }
                keys[KEY_ACTION] = 1;
                break;
            case SDL_BUTTON_RIGHT:
                keys[KEY_ACTION + 1] = 1;
                break;
            }
        }
        if(e.type == SDL_MOUSEBUTTONUP) {
            switch (e.button.button) {
            case SDL_BUTTON_LEFT:
                keys[KEY_ACTION] = 0;
                break;
            case SDL_BUTTON_RIGHT:
                keys[KEY_ACTION + 1] = 0;
                break;
            }
        }
        if(e.type == SDL_MOUSEWHEEL) {
            if (e.wheel.y > 0) { // up
                keys[KEY_PREV] = 1;
            }
            if (e.wheel.y < 0) { // down
                keys[KEY_NEXT] = 1;
            }
        }
        if(e.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode() == true) {
            mouse_x += e.motion.xrel;
            mouse_y += e.motion.yrel;
        }
        if(e.type == SDL_WINDOWEVENT) {
            if(e.window.event == SDL_WINDOWEVENT_FOCUS_LOST && SDL_GetRelativeMouseMode() == true) {
                SDL_SetRelativeMouseMode(false);
            }
            if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                r_current_window_width = e.window.data1;
                r_current_window_height = e.window.data2;
                float fw = r_current_window_width;
                float fh = r_current_window_height;
                float dw = INTERNAL_W;
                float dh = INTERNAL_H;
                float ratio;
                r_pady = 0;
                r_padx = 0;
                if (fw / dw >= fh / dh) {
                    ratio = fh / dh;
                    r_padx = (fw - (dw * ratio)) / 2.0f;
                } else {
                    ratio = fw / dw;
                    r_pady = (fh - (dh * ratio)) / 2.0f;
                }
            }
        }
    }

}