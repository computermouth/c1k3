
#include <stdint.h>
#include <stdio.h>

#include "entity_light.h"
#include "game.h"
#include "math.h"
#include "render.h"

void entity_light_init(entity_t * e, uint8_t light, uint8_t color);
void entity_light_update(entity_t * e);

void entity_light_constructor(entity_t * e, vec3_t pos, uint8_t light, uint8_t color) {
    entity_constructor(e, pos, light, color);

    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_light_init;
    e->_update = (void (*)(void *))entity_light_update;

    e->_init(e, light, color);
}

void entity_light_init(entity_t * e, uint8_t light, uint8_t color) {
    e->_light = light;
    e->_spawn_time = game_time;
    if (light == 1)
        e->_flicker = true;
    if (!color) // todo, real log
        printf("e: no color\n");
    e->_color[0] = ((color & 0x7) << 5);
    e->_color[1] = ((color & 0x1c) << 3);
    e->_color[2] = (color & 0xc0);
}

void entity_light_update(entity_t * e) {
    if (e->_flicker && randf() > 0.9)
        e->_light = randf() > 0.5 ? 10 : 0;
    uint32_t intensity = e->_light;

    // If this light is a temporary one, fade it out over its lifetime
    if (e->_expires) {
        if (e->_die_at < game_time) {
            e->_kill(e);
        }
        intensity = scale(game_time, e->_spawn_time, e->_die_at, 1, 0) * e->_light;
    }

    r_push_light(e->p, intensity, e->_color[0], e->_color[1], e->_color[2]);
}