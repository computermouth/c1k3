
#ifndef _MATH_
#define _MATH_

typedef struct {
    float x;
    float y;
    float z;
} vec3_t;

float randf();
float clamp (float v, float min, float max);
float scale (float v, float in_min, float in_max, float out_min, float out_max);
float anglemod(float r);
// TODO remove both of these
vec3_t vec3(float x, float y, float z);
vec3_t vec3_clone(vec3_t a);
// TODO, change a lot of these to void(), pass pointers and modify
vec3_t vec3_rotate_y(vec3_t p, float rad);
vec3_t vec3_rotate_x(vec3_t p, float rad);
vec3_t vec3_rotate_yaw_pitch(vec3_t p, float yaw, float pitch);
float vec3_2d_angle(vec3_t a, vec3_t b);
float vec3_length(vec3_t a);
vec3_t vec3_sub(vec3_t a, vec3_t b);
float vec3_dist(vec3_t a, vec3_t b);
float vec3_dot(vec3_t a, vec3_t b);
vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_mul(vec3_t a, vec3_t b);
vec3_t vec3_mulf(vec3_t a, float b);
vec3_t vec3_cross(vec3_t a, vec3_t b);
vec3_t vec3_normalize(vec3_t v);
vec3_t vec3_face_normal(vec3_t v0, vec3_t v1, vec3_t v2);


#endif