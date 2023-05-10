
#ifndef _TEXT_
#define _TEXT_

#include <stdint.h>

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
    int x;
    int y;
} font_banner_t;

void text_init();
void text_overlay_resize();
void * text_push_banner(font_banner_t banner);
int text_update_banner(void * banner_ptr);
int text_pop_banner(void * banner_ptr);
void text_render_overlay();
void text_pop_all();
void text_free();

#endif