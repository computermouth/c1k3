
#include <stdio.h>
#include <stdint.h>

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
uint32_t r_num_lights = 0;

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