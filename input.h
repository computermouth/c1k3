
#ifndef _INPUT_
#define _INPUT_

#include <stdbool.h>

typedef enum {
    _KEY_NULL,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_PREV,
    KEY_NEXT,
    KEY_ACTION,
    KEY_JUMP,
    _KEY_END
} key_enum_t;

extern float mouse_x;
extern float mouse_y;
extern float last_wheel_event;
extern int mouse_speed;
extern bool mouse_invert;
extern bool input_quit;
extern bool keys[_KEY_END];

void input_consume();

#endif