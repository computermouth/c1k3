
#include <math.h>

#include "math.h" // shit, rename

float math_clamp (float v, float min, float max){
    return v < min ? min : (v > max ? max : v);
}

float math_scale (float v, float in_min, float in_max, float out_min, float out_max){
    return out_min + ((out_max) - out_min) * (((v) - in_min) / ((in_max) - in_min));
}

float math_anglemod(float r) {
    return atan2f(sinf(r), cosf(r));
}

// TODO remove both of these
vec3_t vec3(float x, float y, float z){
    return (vec3_t){.x = x, .y = y, .z = z};
}
vec3_t vec3_clone(vec3_t a){
    return (vec3_t){.x = a.x, .y = a.y, .z = a.z};
}

// TODO, change a lot of these to void(), pass pointers and modify
vec3_t vec3_rotate_y(vec3_t p, float rad){
    return (vec3_t){p.z * sinf(rad) + p.x * cosf(rad), p.y, p.z * cosf(rad) - p.x * sinf(rad)};
}

vec3_t vec3_rotate_x(vec3_t p, float rad){    
    return (vec3_t){p.x, p.y * cosf(rad) - p.z * sinf(rad), p.y * sinf(rad) + p.z * cosf(rad)};
}

vec3_t vec3_rotate_yaw_pitch(vec3_t p, float yaw, float pitch){
    return vec3_rotate_y(vec3_rotate_x(p, pitch), yaw);
}

float vec3_2d_angle(vec3_t a, vec3_t b){
    return atan2f(b.x - a.x, b.z - a.z);
}

float vec3_length(vec3_t a){
    // C doesn't have a 3-argument hypot()
    return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}

vec3_t vec3_sub(vec3_t a, vec3_t b){
    return (vec3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

float vec3_dist(vec3_t a, vec3_t b){
    return vec3_length(vec3_sub(a, b));
}

float vec3_dot(vec3_t a, vec3_t b){
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

vec3_t vec3_add(vec3_t a, vec3_t b){
    return (vec3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3_t vec3_mul(vec3_t a, vec3_t b){
    return (vec3_t){a.x * b.x, a.y * b.y, a.z * b.z};
}

vec3_t vec3_mulf(vec3_t a, float b){
    return (vec3_t){a.x * b, a.y * b, a.z * b};
}

vec3_t vec3_cross(vec3_t a, vec3_t b){
    return (vec3_t){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

vec3_t vec3_normalize(vec3_t v){
    return vec3_mulf(v, 1/vec3_length(v));
}

vec3_t vec3_face_normal(vec3_t v0, vec3_t v1, vec3_t v2){
    return vec3_normalize(vec3_cross(vec3_sub(v0, v1), vec3_sub(v2, v1)));
}