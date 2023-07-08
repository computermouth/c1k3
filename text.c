
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "text.h"
#include "render.h"
#include "data.h"
#include "vector.h"

typedef struct {
    text_surface_t * ts;
    float end_time;
} _internal_timed_surface_t;

SDL_Surface * overlay_surface = NULL;
SDL_RWops * font_sm_rw = NULL;
SDL_RWops * font_md_rw = NULL;
SDL_RWops * font_lg_rw = NULL;
TTF_Font * font_sm = NULL;
TTF_Font * font_md = NULL;
TTF_Font * font_lg = NULL;

vector * timed_surfaces = NULL;

/*
SDL_Surface* create_surface() {
    SDL_Color c = {.r = 255, .g = 255, .b = 255, .a = 255};
    SDL_Surface* ammo = TTF_RenderUTF8_Solid(font_md, "ammo: ∞", c);
    SDL_Surface* health = TTF_RenderUTF8_Solid(font_md, "♥: 50", c);
    SDL_Surface* c1k3 = TTF_RenderUTF8_Solid(font_lg, "C1K3", c);
    SDL_Surface* dq = TTF_RenderUTF8_Solid(font_sm, "-- dequake fps --", c);

    int width = INTERNAL_W;
    int height = INTERNAL_H;

    // todo, endianness
    SDL_Surface* surface =SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ABGR8888);
    SDL_FillRect(surface, 0, 0x00000000);

    SDL_BlitSurface(ammo, NULL, surface, &(SDL_Rect) {
        .x = (width / 6) - ammo->w / 2,
        .y = height - ammo->h - 10,
    });
    SDL_FreeSurface(ammo);

    SDL_BlitSurface(health, NULL, surface, &(SDL_Rect) {
        .x = (width / 6 * 5) - health->w / 2,
        .y = height - health->h - 10,
    });
    SDL_FreeSurface(health);

    SDL_BlitSurface(c1k3, NULL, surface, &(SDL_Rect) {
        .x = (width / 2) - c1k3->w / 2,
        .y = (height / 2) - c1k3->h / 2,
    });

    SDL_BlitSurface(dq, NULL, surface, &(SDL_Rect) {
        .x = (width / 2) - dq->w / 2,
        .y = (height / 2) + dq->h,
    });
    SDL_FreeSurface(dq);
    SDL_FreeSurface(c1k3);

    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create SDL surface: %s", SDL_GetError());
        return NULL;
    }
    return surface;
}
*/

GLuint overlay_program;
GLuint overlay_position;
GLuint overlay_texcoord;
GLuint overlay_tex;
GLuint overlay_vbo;
GLuint overlay_texture;
void text_init() {

    timed_surfaces = vector_init(sizeof(_internal_timed_surface_t));

    TTF_Init();

    // this is kinda goofy, but alright
    font_sm_rw = SDL_RWFromMem((void *)data_terminess_font, data_terminess_font_len);
    font_sm = TTF_OpenFontRW(font_sm_rw, 0, 12);
    font_md_rw = SDL_RWFromMem((void *)data_terminess_font, data_terminess_font_len);
    font_md = TTF_OpenFontRW(font_md_rw, 0, 18);
    font_lg_rw = SDL_RWFromMem((void *)data_terminess_font, data_terminess_font_len);
    font_lg = TTF_OpenFontRW(font_lg_rw, 0, 32);

    // Create a shader program and get the attribute and uniform locations
    GLuint vertex_shader = r_compile_shader(
                               GL_VERTEX_SHADER,
                               "#version 300 es\n"
                               "in vec4 position;\n"
                               "in vec2 texcoord;\n"
                               "out vec2 v_texcoord;\n"
                               "void main()\n"
                               "{\n"
                               "    gl_Position = position;\n"
                               "    v_texcoord = texcoord;\n"
                               "}\n");
    GLuint fragment_shader = r_compile_shader(
                                 GL_FRAGMENT_SHADER,
                                 "#version 300 es\n"
                                 "precision mediump float;\n"
                                 "in vec2 v_texcoord;\n"
                                 "uniform sampler2D tex;\n"
                                 "out vec4 color;\n"
                                 "void main()\n"
                                 "{\n"
                                 "    color = texture(tex, v_texcoord);\n"
                                 "}\n");

    overlay_program = r_create_program(vertex_shader, fragment_shader);
    glUseProgram(overlay_program);
    overlay_position = glGetAttribLocation(overlay_program, "position");
    overlay_texcoord = glGetAttribLocation(overlay_program, "texcoord");
    overlay_tex = glGetUniformLocation(overlay_program, "tex");

    GLfloat vertices[] = {
        // bottom right
        // Position       Texture coordinates
        1.0f, -1.0f,     1.0f, 1.0f,
        -1.0f, -1.0f,     0.0f, 1.0f,
        1.0f,  1.0f,     1.0f, 0.0f,
        // top left
        // Position       Texture coordinates
        -1.0f,  1.0f,     0.0f, 0.0f,
        1.0f,  1.0f,     1.0f, 0.0f,
        -1.0f, -1.0f,     0.0f, 1.0f,
    };

    // Create a VBO and upload the vertex data
    glGenBuffers(1, &overlay_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, overlay_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // todo, endianness
    overlay_surface =SDL_CreateRGBSurfaceWithFormat(0, INTERNAL_W, INTERNAL_H, 32, SDL_PIXELFORMAT_ABGR8888);
    SDL_FillRect(overlay_surface, 0, 0x00000000);

    // Create a GL texture from the surface
    glGenTextures(1, &overlay_texture);
    glBindTexture(GL_TEXTURE_2D, overlay_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, overlay_surface->w, overlay_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, overlay_surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void text_end_frame() {

    uint32_t len = vector_size(timed_surfaces);
    for(uint32_t i = 0; i < len; i++) {
        _internal_timed_surface_t * timed_surf = vector_at(timed_surfaces, i);
        if (timed_surf->end_time < game_time) {
            text_free_surface(timed_surf->ts);
            vector_erase(timed_surfaces, i);
            i--;
            len--;
        } else {
            text_surface_t * ts = timed_surf->ts;
            text_push_surface(ts);
        }
    }

    glBindTexture(GL_TEXTURE_2D, overlay_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, overlay_surface->w, overlay_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, overlay_surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glDisable(GL_CULL_FACE);
    glUseProgram(overlay_program);
    glBindBuffer(GL_ARRAY_BUFFER, overlay_vbo);

    glEnableVertexAttribArray(overlay_position);
    glEnableVertexAttribArray(overlay_texcoord);
    glVertexAttribPointer(overlay_position, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
    glVertexAttribPointer(overlay_texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (GLvoid*)(sizeof(GLfloat) * 2));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, overlay_texture);
    glUniform1i(overlay_tex, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_CULL_FACE);
}

text_surface_t * text_create_surface(font_input_t input) {

    TTF_Font * font = NULL;
    switch (input.size) {
    case FONT_SM:
        font = font_sm;
        break;
    case FONT_MD:
        font = font_md;
        break;
    case FONT_LG:
        font = font_lg;
        break;
    default:
        fprintf(stderr, "E: unexpected font size, skipping..\n");
        return NULL;
    }

    SDL_Color fg = {.r = input.color.r, .g = input.color.g, .b = input.color.b, .a = input.color.a};
    SDL_Surface * tmp_fg = TTF_RenderUTF8_Solid(font, input.text, fg);

    // // todo maybe later
    //
    // SDL_Color bg = {.r = banner.bg.r, .g = banner.bg.g, .b = banner.bg.b, .a = banner.bg.a};
    // SDL_Surface * tmp_bg = SDL_CreateRGBSurfaceWithFormat(0, tmp_fg->w, tmp_fg->h, 32, SDL_PIXELFORMAT_ABGR8888);
    // SDL_FillRect(tmp_bg, 0, bg.r << 24 | bg.g << 16 | bg.b << 8 | bg.a );

    // SDL_BlitSurface(tmp_fg, 0, tmp_bg, 0);
    // SDL_FreeSurface(tmp_fg);

    text_surface_t * ts = malloc(sizeof(text_surface_t));
    ts->w = tmp_fg->w;
    ts->h = tmp_fg->h;
    ts->x = 0;
    ts->y = 0;
    ts->data = tmp_fg;

    return ts;
}

void text_prepare_frame() {
    SDL_FillRect(overlay_surface, 0, 0x00000000);
}

void text_push_timed_surface(timed_surface_t time_surf) {
    _internal_timed_surface_t its = {.ts = time_surf.ts, .end_time = game_time + ((float)time_surf.ms / 1000)};
    vector_push(timed_surfaces, &its);
}

void text_push_surface(text_surface_t * ts) {
    SDL_BlitSurface(ts->data, NULL, overlay_surface, &(SDL_Rect) {
        .x = ts->x,
        .y = ts->y,
    });
}

void text_free_surface(text_surface_t * ts) {
    if(ts->data)
        SDL_FreeSurface(ts->data);
    if(ts)
        free(ts);
}

void text_quit() {

    uint32_t len = vector_size(timed_surfaces);
    for(uint32_t i = 0; i < len; i++) {
        _internal_timed_surface_t * timed_surf = vector_at(timed_surfaces, i);
        text_free_surface(timed_surf->ts);
    }
    vector_free(timed_surfaces);

    // todo
    SDL_FreeSurface(overlay_surface);
    // apparently this fails to free, as it's
    // data block memory
    TTF_CloseFont(font_sm);
    TTF_CloseFont(font_md);
    TTF_CloseFont(font_lg);
    SDL_FreeRW(font_sm_rw);
    SDL_FreeRW(font_md_rw);
    SDL_FreeRW(font_lg_rw);
}
