#include <GLES3/gl3.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "math.h"
#include "data.h"
#include "render.h"
#include "lodepng.h"

typedef struct {
    int antialias;
} options_t;
options_t options = { .antialias = 0 };

#define R_MAX_VERTS (1024 * 64)
#define R_MAX_LIGHT_V3 64

// god forgive me
#define STR_PREP(x) #x
#define STR(x) STR_PREP(x)

char * SHADER_VERT =
#include "shader.vert"
    ;

char * SHADER_FRAG =
#include "shader.frag"
    ;

// 8 properties per vert [x,y,z, u,v, nx,ny,nz]
float r_buffer[R_MAX_VERTS*8] = {0};
int r_num_verts = 0;

// 2 vec3 per light [(x,y,z), [r,g,b], ...]
float r_light_buffer[R_MAX_LIGHT_V3*3] = {0};
uint32_t r_num_lights = 0;

// Uniform locations
GLint r_u_camera;
GLint r_u_lights;
GLint r_u_mouse;
GLint r_u_pos;
GLint r_u_rotation;
GLint r_u_frame_mix;

// Vertex attribute location for mixing
GLint r_va_p2;
GLint r_va_n2;

// Texture handles
meta_tex_t * r_textures = NULL;
uint32_t num_textures = 0;

// Camera position
vec3_t r_camera = { .x = 0, .y = 0, .z = -50};
GLfloat r_camera_pitch = 0.2;
GLfloat r_camera_yaw = 0;

// We collect all draw calls in an array and draw them all at once at the end
// the frame. This way the lights buffer will be completely filled and we
// only need to set it once for all geometry
draw_call_t * r_draw_calls = NULL;
uint32_t r_num_draw_calls = 0;

// we render to an offscreen buffer, so we can blit
// and retain internal resolution and aspect ratio
GLint default_fbo;
GLuint offscreen_fbo;
GLuint offscreen_color_tex;
GLuint offscreen_depth_tex;

int32_t r_padx = 0;
int32_t r_pady = 0;
int32_t r_current_window_width = D_WINDOW_W;
int32_t r_current_window_height = D_WINDOW_H;

GLuint r_compile_shader(GLenum type, char* source) {

    GLuint shader = glCreateShader(type);
    const GLchar *g[] = {source};
    glShaderSource(shader, 1, g, NULL);
    glCompileShader(shader);
    GLchar infoLog[501];
    GLsizei length;
    glGetShaderInfoLog(shader, 500, &length, infoLog);
    if (strcmp(infoLog, ""))
        printf("compiler: %s\n", infoLog);
    return shader;
}

GLint r_vertex_attrib(GLuint shader_program, const GLchar *attrib_name, int count, int vertex_size, long int offset) {
    GLint location = glGetAttribLocation(shader_program, attrib_name);
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, count, GL_FLOAT, GL_FALSE, vertex_size*4,(void *)(offset * 4));
    return location;
}


GLuint vertex_buffer;
void r_init() {

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, r_compile_shader(GL_VERTEX_SHADER, SHADER_VERT));
    glAttachShader(shader_program, r_compile_shader(GL_FRAGMENT_SHADER, SHADER_FRAG));
    glLinkProgram(shader_program);
    glUseProgram(shader_program);

    r_u_camera = glGetUniformLocation(shader_program, "c");
    r_u_lights = glGetUniformLocation(shader_program, "l");
    r_u_mouse = glGetUniformLocation(shader_program, "m");
    r_u_pos = glGetUniformLocation(shader_program, "mp");
    r_u_rotation = glGetUniformLocation(shader_program, "mr");
    r_u_frame_mix = glGetUniformLocation(shader_program, "f");

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    r_vertex_attrib(shader_program, "p", 3, 8, 0);
    r_vertex_attrib(shader_program, "t", 2, 8, 3);
    r_vertex_attrib(shader_program, "n", 3, 8, 5);

    r_va_p2 = r_vertex_attrib(shader_program, "p2", 3, 8, 0);
    r_va_n2 = r_vertex_attrib(shader_program, "n2", 3, 8, 5);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);

    glViewport(0, 0, INTERNAL_W, INTERNAL_H);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // save default fbo
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &default_fbo);

    // initialize offscreen fbo
    glGenFramebuffers(1, &offscreen_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, offscreen_fbo);

    // initialize backing texture for offscreen fbo
    glGenTextures(1, &offscreen_color_tex);
    glBindTexture(GL_TEXTURE_2D, offscreen_color_tex);
    // todo, window dimensions??
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, INTERNAL_W, INTERNAL_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscreen_color_tex, 0);

    // initialize depth texture for offscreen fbo
    glGenTextures(1, &offscreen_depth_tex);
    glBindTexture(GL_TEXTURE_2D, offscreen_depth_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, INTERNAL_W, INTERNAL_H, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // todo, not sure
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, offscreen_depth_tex, 0);
}

void r_create_texture(png_bin_t p) {

    uint32_t width;
    uint32_t height;
    uint8_t *img;
    uint32_t error;

    // todo, port picopng?
    error = lodepng_decode32(&img, &width, &height, p.data, p.len);
    if(error) printf("lodepng error %u: %s\n", error, lodepng_error_text(error));

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);

    // todo, vectorize?
    num_textures++;
    r_textures = realloc(r_textures, sizeof(meta_tex_t) * num_textures);
    r_textures[num_textures - 1] = (meta_tex_t) {
        .texture = texture,
        .width = width,
        .height = height
    };

    free(img);

}

void r_prepare_frame(float r, float g, float b) {
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    r_num_lights = 0;
    memset(r_light_buffer, 0.0f, R_MAX_LIGHT_V3*3 * sizeof(float));
}

void r_end_frame() {

    glBindFramebuffer(GL_FRAMEBUFFER, offscreen_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform4f(r_u_camera, r_camera.x, r_camera.y, r_camera.z, 16.0f/9.0f);
    glUniform2f(r_u_mouse, r_camera_yaw, r_camera_pitch);
    glUniform3fv(r_u_lights, r_num_lights * 6, r_light_buffer);

    long int vo = 0;
    int last_texture = -1;

    for(uint32_t i = 0; i < r_num_draw_calls; i++) {
        draw_call_t c = r_draw_calls[i];

        if (last_texture != c.texture) {
            last_texture = c.texture;
            glBindTexture(GL_TEXTURE_2D, r_textures[last_texture].texture);
        }

        glUniform3f(r_u_pos, c.pos.x, c.pos.y, c.pos.z);
        glUniform2f(r_u_rotation, c.yaw, c.pitch);
        glUniform1f(r_u_frame_mix, c.mix);

        if (vo != (c.f2 - c.f1)) {
            vo = (c.f2 - c.f1);
            glVertexAttribPointer(r_va_p2, 3, GL_FLOAT, GL_FALSE, 8 * 4, (void *)(vo*8*4));
            glVertexAttribPointer(r_va_n2, 3, GL_FLOAT, GL_FALSE, 8 * 4, (void *)((vo*8+5)*4));
        }
        glDrawArrays(GL_TRIANGLES, c.f1, c.num_verts);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, default_fbo);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, offscreen_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, default_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glBlitFramebuffer(0, 0, WINDOW_W, WINDOW_H, padx, pady, w - padx, h - pady, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, 0, INTERNAL_W, INTERNAL_H, r_padx, r_pady, r_current_window_width - r_padx, r_current_window_height - r_pady, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, 0, INTERNAL_W, INTERNAL_H, r_padx, r_pady, r_current_window_width - r_padx, r_current_window_height - r_pady, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    // memset(r_draw_calls, 0, sizeof(draw_call_t) * r_num_draw_calls);
    free(r_draw_calls);
    r_draw_calls = NULL;
    r_num_draw_calls = 0;
}

void r_draw(draw_call_t call) {
    r_num_draw_calls++;
    r_draw_calls = realloc(r_draw_calls, sizeof(draw_call_t) * r_num_draw_calls);
    r_draw_calls[r_num_draw_calls - 1] = call;
}

void r_submit_buffer() {
    glBufferData(GL_ARRAY_BUFFER, r_num_verts * 8 * sizeof(float), r_buffer, GL_STATIC_DRAW);
}

void r_push_vert(vec3_t pos, vec3_t normal, float u, float v) {
    // todo, memcpy?
    r_buffer[r_num_verts*8 + 0] = pos.x;
    r_buffer[r_num_verts*8 + 1] = pos.y;
    r_buffer[r_num_verts*8 + 2] = pos.z;
    r_buffer[r_num_verts*8 + 3] = u;
    r_buffer[r_num_verts*8 + 4] = v;
    r_buffer[r_num_verts*8 + 5] = normal.x;
    r_buffer[r_num_verts*8 + 6] = normal.y;
    r_buffer[r_num_verts*8 + 7] = normal.z;

    r_num_verts++;
}

void r_push_quad(vec3_t v0, vec3_t v1, vec3_t v2, vec3_t v3, float u, float v) {
    vec3_t n = vec3_face_normal(v0, v1, v2);
    r_push_vert(v0, n, u, 0);
    r_push_vert(v1, n, 0, 0);
    r_push_vert(v2, n, u, v);
    r_push_vert(v3, n, 0, v);
    r_push_vert(v2, n, u, v);
    r_push_vert(v1, n, 0, 0);
}

int r_push_block(float x, float y, float z, float sx, float sy, float sz, int texture) {

    uint32_t tex_w = r_textures[texture].width;
    uint32_t tex_h = r_textures[texture].height;
    int index = r_num_verts;

    float tx = sx/tex_w;
    float ty = sy/tex_h;
    float tz = sz/tex_w;

    // top
    vec3_t v0 = vec3(x, y + sy, z);
    vec3_t v1 = vec3(x + sx, y + sy, z);
    vec3_t v2 = vec3(x, y + sy, z + sz);
    vec3_t v3 = vec3(x + sx, y + sy, z + sz);

    // bottom
    vec3_t v4 = vec3(x, y, z + sz);
    vec3_t v5 = vec3(x + sx, y, z + sz);
    vec3_t v6 = vec3(x, y, z);
    vec3_t v7 = vec3(x + sx, y, z);

    r_push_quad(v0, v1, v2, v3, tx, tz); // top
    r_push_quad(v4, v5, v6, v7, tx, tz); // bottom
    r_push_quad(v2, v3, v4, v5, tx, ty); // front
    r_push_quad(v1, v0, v7, v6, tx, ty); // back
    r_push_quad(v3, v1, v5, v7, tz, ty); // right
    r_push_quad(v0, v2, v6, v4, tz, ty); // left
    return index;
}

void r_push_light(vec3_t pos, float intensity, float r, float g, float b) {
    // Calculate the distance to the light, fade it out between 768--1024
    float fade = clamp(
                     scale(
                         vec3_dist(pos, r_camera),
                         768, 1024, 1, 0
                     ),
                     0, 1
                 ) * intensity * 10;

    if (fade && r_num_lights < R_MAX_LIGHT_V3/2) {

        // todo, memcpy?
        r_light_buffer[r_num_lights*6 + 0] = pos.x;
        r_light_buffer[r_num_lights*6 + 1] = pos.y;
        r_light_buffer[r_num_lights*6 + 2] = pos.z;
        r_light_buffer[r_num_lights*6 + 3] = r*fade;
        r_light_buffer[r_num_lights*6 + 4] = g*fade;
        r_light_buffer[r_num_lights*6 + 5] = b*fade;

        r_num_lights++;
    }
};