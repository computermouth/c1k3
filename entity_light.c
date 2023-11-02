
#include <stdint.h>
#include <stdio.h>

#include "entity_light.h"
#include "game.h"
#include "map.h"
#include "math.h"
#include "render.h"

void entity_light_init(entity_t * e);
void entity_light_update(entity_t * e);

void entity_light_constructor(entity_t * e) {
    entity_constructor(e);

    e->_update = entity_light_update;

    entity_light_init(e);
}

void entity_light_init(entity_t * e) {
    e->_light = e->_params->entity_light_params.rgba[3];
    e->_spawn_time = game_time;
    if (e->_light == 1)
        e->_flicker = true;

    e->_color[0] = e->_params->entity_light_params.rgba[0];
    e->_color[1] = e->_params->entity_light_params.rgba[1];
    e->_color[2] = e->_params->entity_light_params.rgba[2];

    e->_color[0] -= e->_color[0] % 16;
    e->_color[1] -= e->_color[1] % 16;
    e->_color[2] -= e->_color[2] % 16;

    if (!e->_color[0] && !e->_color[1] && !e->_color[2]) // todo, real log
        printf("e: no color\n");
}

void entity_light_update(entity_t * e) {
    if (e->_flicker && randf() > 0.9)
        e->_light = randf() > 0.5 ? 10 : 0;
    float intensity = e->_light;

    // If this light is a temporary one, fade it out over its lifetime
    if (e->_expires) {
        if (e->_die_at < game_time) {
            e->_kill(e);
        }
        intensity = scale(game_time, e->_spawn_time, e->_die_at, 1, 0) * e->_light;
    }

    r_push_light(e->p, intensity, e->_color[0], e->_color[1], e->_color[2]);
}