
#include <math.h>

#include "entity_torch.h"
#include "entity.h"
#include "game.h"
#include "math.h"
#include "map.h"
#include "render.h"

uint32_t torch_anim_frames[] = {0,1,2,1,2,0,0,1,2,1,2,1,2,0};
animation_t torch_animation = {
    .time = 0.25,
    .frames = torch_anim_frames,
    .num_frames = sizeof(torch_anim_frames)/sizeof(torch_anim_frames[0]),
};

void entity_torch_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_torch_update(entity_t * e);

void entity_torch_constructor(entity_t * e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_constructor(e, pos, p1, p2);

    e->_update = entity_torch_update;

    entity_torch_init(e, p1, p2);
}

void entity_torch_init(entity_t * e, uint8_t p1, uint8_t p2) {

    e->_anim = &torch_animation;

    e->p.x -= 16;
    e->p.z -= 16;
    e->_light_pos = e->p;

    vec3_t v[] = {
        vec3(-32,0,0),
        vec3(32,0,0),
        vec3(0,0,-32),
        vec3(0,0,32)
    };

    // Find which wall we're on; move the torch model towards the wall and
    // the light position outwards
    for(uint32_t i = 0; i < 4; i++) {
        vec3_t trace_dir = v[i];
        vec3_t trace_end = vec3_add(e->p, trace_dir);
        if(map_trace(e->p, trace_end)) {
            e->p = vec3_add(e->p, vec3_mulf(trace_dir, 0.4));
            e->_light_pos = vec3_sub(e->p, vec3_mulf(trace_dir, 2));
            break;
        }
    }

    e->_light = 0;

    entity_set_model(e);
}

void entity_torch_update(entity_t * e) {
    entity_update(e);
    if (randf() > 0.8) {
        e->_light = randf();
    }
    r_push_light(e->_light_pos, sinf(game_time) + e->_light + 6, 255,192,16);
}