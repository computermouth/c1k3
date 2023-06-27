
#include <GLES2/gl2.h>
#include <SDL2/SDL_rwops.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <GLES3/gl3.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "text.h"
#include "render.h"
#include "data.h"

/*
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} font_color_t;

typedef enum {
    FONT_SM,
    FONT_MD,
    FONT_LG
} font_size_t;

typedef struct {
    char * input;
    font_color_t fg;
    font_color_t bg;
    font_size_t size;
    int x;
    int y;
} font_banner_t;
*/

typedef struct {
    font_banner_t banner;
    SDL_Surface * surface;
    int x;
    int y;
} _internal_font_banner_t;

typedef struct {
    _internal_font_banner_t ** internal_banners;
    bool updated;
    uint32_t capacity;
    uint32_t len;
} _internal_banner_vector;

_internal_banner_vector ibv = {
    .internal_banners = NULL,
    .updated = false,
    .capacity = 0,
    .len = 0,
};

SDL_Surface * overlay_surface = NULL;
SDL_RWops * font_sm_rw = NULL;
SDL_RWops * font_md_rw = NULL;
SDL_RWops * font_lg_rw = NULL;
TTF_Font * font_sm = NULL;
TTF_Font * font_md = NULL;
TTF_Font * font_lg = NULL;

void update_surface() {

    if (!ibv.updated)
        return;

    for(int i = 0; i < ibv.len; i++) {


        // blit to overlay
    }

}

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

    TTF_Init();

    // initialize banner vector
    ibv.capacity = 8;
    ibv.internal_banners = calloc(ibv.capacity, sizeof(_internal_font_banner_t *));

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

void text_render_overlay() {
    update_surface();

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

void text_raster_banner(uint32_t i) {

    if (i >= ibv.len) {
        fprintf(stderr, "E: invalid banner specified\n");
        return;
    }
    _internal_font_banner_t * ib = ibv.internal_banners[i];
    font_banner_t banner = ib->banner;

    TTF_Font * font = NULL;
    switch (banner.size) {
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
        return;
    }

    SDL_Color fg = {.r = banner.fg.r, .g = banner.fg.g, .b = banner.fg.b, .a = banner.fg.a};
    SDL_Surface * tmp_fg = TTF_RenderUTF8_Solid(font, banner.input, fg);

    SDL_Color bg = {.r = banner.bg.r, .g = banner.bg.g, .b = banner.bg.b, .a = banner.bg.a};
    SDL_Surface * tmp_bg = SDL_CreateRGBSurfaceWithFormat(0, tmp_fg->w, tmp_fg->h, 32, SDL_PIXELFORMAT_ABGR8888);
    SDL_FillRect(tmp_bg, 0, bg.r << 24 | bg.g << 16 | bg.b << 8 | bg.a );

    SDL_BlitSurface(tmp_fg, 0, tmp_bg, 0);
    SDL_FreeSurface(tmp_fg);

    if (ib->surface != NULL)
        SDL_FreeSurface(ib->surface);

    ib->surface = tmp_bg;
}

void * text_push_banner(font_banner_t banner) {
    if (banner.input == NULL) {
        fprintf(stderr, "E: banner.input cannot be null");
        return NULL;
    }

    ibv.len++;
    if (ibv.len == ibv.capacity) {
        ibv.capacity *= 2;
        ibv.internal_banners = realloc(ibv.internal_banners, sizeof(_internal_font_banner_t *) * ibv.capacity);
        memset(&(ibv.internal_banners[ibv.capacity/2]), 0, sizeof(_internal_font_banner_t *) * ibv.capacity/2);
    }

    uint32_t index = ibv.len - 1;
    ibv.internal_banners[index] = calloc(1, sizeof(_internal_font_banner_t));
    font_banner_t * ib = &(ibv.internal_banners[index]->banner);
    *ib = (font_banner_t) {
        .input = NULL,
        .bg = banner.bg,
        .fg = banner.fg,
        .size = banner.size,
        .x = banner.x,
        .y = banner.y,
    };

    uint32_t input_len = strlen(banner.input) + 1;
    ib->input = calloc(input_len, sizeof(char));
    strncpy(ib->input, banner.input, input_len);

    text_raster_banner(index);

    return ib;
}

int text_update_banner(void * banner_ptr, font_banner_t banner) {

    for(uint32_t i = 0; i < ibv.len; i++) {
        if (ibv.internal_banners[i] == banner_ptr) {
            free(ibv.internal_banners[i]->banner.input);

            ibv.internal_banners[i]->banner = banner;

            uint32_t input_len = strlen(banner.input) + 1;
            ibv.internal_banners[i]->banner.input = calloc(input_len, sizeof(char));
            strncpy(ibv.internal_banners[i]->banner.input, banner.input, input_len);

            text_raster_banner(i);
            return 0;
        }
    }

    fprintf(stderr, "E: unable to update banner -- not found");
    return 1;
}

int text_pop_banner(void * banner_ptr) {
    for(uint32_t i = 0; i < ibv.len; i++) {
        if (ibv.internal_banners[i] == banner_ptr) {
            free(ibv.internal_banners[i]->banner.input);
            free(ibv.internal_banners[i]);
            free(ibv.internal_banners[i]->surface);
            free(ibv.internal_banners[i]);
            ibv.internal_banners[i] = NULL;

            if(i == ibv.len - 1) {
                fprintf(stderr, "removed last item\n");
                return 0;
            }

            memmove(&(ibv.internal_banners[i]), &(ibv.internal_banners[i + 1]), i - (ibv.len - 1) );
            ibv.len--;
            return 0;
        }
    }

    fprintf(stderr, "E: unable to pop banner -- not found");
    return 1;
}

void text_pop_all() {
    for(uint32_t i = 0; i < ibv.len; i++) {
        free(ibv.internal_banners[i]->banner.input);
        free(ibv.internal_banners[i]);
        free(ibv.internal_banners[i]->surface);
        free(ibv.internal_banners[i]);
        ibv.internal_banners[i] = NULL;
    }

    ibv.len = 0;
}

void text_free() {

    text_pop_all();
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