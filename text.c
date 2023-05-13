
#include <GLES2/gl2.h>
#include <SDL2/SDL_rwops.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <GLES3/gl3.h>
#include <SDL2/SDL_ttf.h>

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

typedef struct {
    char * input;
    font_color_t fg;
    font_color_t bg;
    float scale;
} font_banner_t;
*/

SDL_Surface * overlay_surface = NULL;
TTF_Font * font_sm = NULL;
TTF_Font * font_md = NULL;
TTF_Font * font_lg = NULL;

bool was_updated = 0;

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

    // // cant scale-blit 8-bit surface to 32-bit surface, so we upgrade first
    // SDL_Surface* tmp_c1k3 =SDL_CreateRGBSurfaceWithFormat(0, c1k3->w, c1k3->h, 32, SDL_PIXELFORMAT_ABGR8888);
    // SDL_FillRect(tmp_c1k3, 0, 0x00000000);
    // SDL_BlitSurface(c1k3, NULL, tmp_c1k3, &(SDL_Rect) {
    //     .x = 0,
    //     .y = 0,
    // });
    // // then scale blit
    // int scale = 4;
    // SDL_BlitScaled(tmp_c1k3, NULL, surface, &(SDL_Rect) {
    //     .x = (surface->w / 2) - tmp_c1k3->w * scale / 2,
    //     .y = 0,
    //     .w = tmp_c1k3->w * scale,
    //     .h = tmp_c1k3->h * scale
    // });
    // SDL_FreeSurface(c1k3);
    // SDL_FreeSurface(tmp_c1k3);

    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create SDL surface: %s", SDL_GetError());
        return NULL;
    }
    return surface;
}

GLuint overlay_program;
GLuint overlay_position;
GLuint overlay_texcoord;
GLuint overlay_tex;
GLuint overlay_vbo;
GLuint overlay_texture;
void text_init() {

    TTF_Init();

    // this is kinda goofy, but alright
    SDL_RWops * font_rw = NULL;
    font_rw = SDL_RWFromMem((void *)data_terminess_font, data_terminess_font_len);
    font_sm = TTF_OpenFontRW(font_rw, 1, 12);
    font_rw = SDL_RWFromMem((void *)data_terminess_font, data_terminess_font_len);
    font_md = TTF_OpenFontRW(font_rw, 1, 18);
    font_rw = SDL_RWFromMem((void *)data_terminess_font, data_terminess_font_len);
    font_lg = TTF_OpenFontRW(font_rw, 1, 32);
    SDL_FreeRW(font_rw);

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

    overlay_surface = create_surface();
    // Create a GL texture from the surface
    glGenTextures(1, &overlay_texture);
    glBindTexture(GL_TEXTURE_2D, overlay_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, overlay_surface->w, overlay_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, overlay_surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void text_render_overlay() {
    if (was_updated) {
        // todo, or something
        was_updated = 0;
    }

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

void text_overlay_resize();
void * text_push_banner(font_banner_t banner);
int text_update_banner(void * banner_ptr);
int text_pop_banner(void * banner_ptr);
void text_pop_all();

void text_free() {
    // todo
    SDL_FreeSurface(overlay_surface);
    // apparently this fails to free, as it's
    // data block memory
    // TTF_CloseFont(font_sm);
    // TTF_CloseFont(font_md);
    // TTF_CloseFont(font_lg);
}