
#include <math.h>

#include "entity_torch.h"
#include "entity.h"
#include "game.h"
#include "math.h"
#include "map.h"
#include "render.h"

animation_t torch_animation[] = {
    {   // 0: Idle
        .time = .25,
        .num_frames = 14,
        .frames = (animation_frame_t[]) {
            {.name = "default"},
            {.name = "torch1"},
            {.name = "torch2"},
            {.name = "torch1"},
            {.name = "torch2"},
            {.name = "default"},
            {.name = "default"},
            {.name = "torch1"},
            {.name = "torch2"},
            {.name = "torch1"},
            {.name = "torch2"},
            {.name = "torch1"},
            {.name = "torch2"},
            {.name = "default"},
        },
    }
};

// hack for caching parsed frame names per-map
static ref_entt_t * last_ref_entt = NULL;


void entity_torch_init(entity_t * e);
void entity_torch_update(entity_t * e);

void entity_torch_constructor(entity_t * e) {
    entity_constructor(e);

    e->_update = entity_torch_update;

    entity_torch_init(e);
}

void entity_torch_init(entity_t * e) {

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

    entity_parse_animation_frames(
        e->_params->entity_generic_params.ref_entt,
        torch_animation,
        sizeof(torch_animation)/sizeof(torch_animation[0]),
        &last_ref_entt
    );

    e->_animation_collection = (animation_collection_t) {
        .animations = torch_animation,
        .num_animations = sizeof(torch_animation)/sizeof(torch_animation[0]),
    };

    e->_anim = &(torch_animation[0]);

    entity_set_model(e);
}

void entity_torch_update(entity_t * e) {
    entity_update(e);
    if (randf() > 0.8) {
        e->_light = randf();
    }
    r_push_light(e->_light_pos, sinf(game_time) + e->_light + 6, 255,192,16);
}