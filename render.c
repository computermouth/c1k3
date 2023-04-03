
#include <stdio.h>

#include "math.h"

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
float r_buffer[R_MAX_VERTS*8];
int r_num_verts = 0;

// 2 vec3 per light [(x,y,z), [r,g,b], ...]
float r_light_buffer[R_MAX_LIGHT_V3*3]; 
int r_num_lights = 0;

// Uniform locations
void * r_u_camera;
void * r_u_lights;
void * r_u_mouse;
void * r_u_pos;
void * r_u_rotation;
void * r_u_frame_mix;

// Vertex attribute location for mixing
void * r_va_p2;
void * r_va_n2;

// Texture handles
void * r_textures[1];

// Camera position
vec3_t r_camera = { .x = 0, .y = 0, .z = -50};
float r_camera_pitch = 0.2;
float r_camera_yaw = 0;

// We collect all draw calls in an array and draw them all at once at the end
// the frame. This way the lights buffer will be completely filled and we
// only need to set it once for all geometry
void * r_draw_calls[1];

void r_init(){
    printf("vert: %s\n", SHADER_VERT);
    printf("frag: %s\n", SHADER_FRAG);
}