
#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL_opengles2.h>
#include "math.h"
#include "data.h"
#include "vector.h"

#define INTERNAL_W 320
#define INTERNAL_H 180
#define D_WINDOW_W 640
#define D_WINDOW_H 360

// todo, verify these all need to be exposed

// We collect all draw calls in an array and draw them all at once at the end
// the frame. This way the lights buffer will be completely filled and we
// only need to set it once for all geometry
typedef struct {
    vec3_t pos;
    float yaw;
    float pitch;
    GLuint texture;
    GLint f1; // todo, first frame of interpolation
    GLint f2; // second frame of interpolation
    float mix;
    int unlit;
    int num_verts;
} draw_call_t;

typedef struct {
    GLuint texture;
    uint32_t width;
    uint32_t height;
} meta_tex_t;

typedef struct {
    uint8_t * data;
    uint64_t len;
} png_bin_t;

// to free on quit
extern vector * r_draw_calls;
extern vector * r_textures;
extern uint32_t r_num_verts;
extern vec3_t r_camera;
extern GLfloat r_camera_pitch;
extern GLfloat r_camera_yaw;

extern int32_t r_padx;
extern int32_t r_pady;
extern int32_t r_current_window_width;
extern int32_t r_current_window_height;

GLuint r_compile_shader(GLenum type, char* source);
GLuint r_create_program(GLuint vertex_shader, GLuint fragment_shader);
GLint r_vertex_attrib(GLuint shader_program, const GLchar *attrib_name, int count, int vertex_size, long int offset);
void r_init();
size_t r_create_texture(png_bin_t p);
void r_prepare_frame(float r, float g, float b);
void r_end_frame();
void r_draw(draw_call_t call);
void r_submit_buffer();
void r_push_vert(vec3_t pos, vec3_t normal, float u, float v);
void r_push_quad(vec3_t v0, vec3_t v1, vec3_t v2, vec3_t v3, float u, float v);
int r_push_block(float x, float y, float z, float sx, float sy, float sz, int texture);
void r_push_light(vec3_t pos, float intensity, float r, float g, float b);
void render_quit();

#endif
