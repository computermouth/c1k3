
#ifndef _TEXT_
#define _TEXT_

#include <stdint.h>

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
    char * text;
    font_color_t color;
    font_size_t size;
} font_input_t;

typedef struct {
    // might want to draw off screen
    // otherwise, minimum size to hold INTERNAL_{W,H}
    // would be fine
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
    void * data;
} text_surface_t;

void text_init();
void text_end_frame();
text_surface_t * text_create_surface(font_input_t input);
void text_prepare_frame();
void text_push_banner(text_surface_t * ts);
void text_free_surface(text_surface_t * ts);
void text_quit();

#endif