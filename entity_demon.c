
#include "entity.h"
#include "game.h"
#include "audio.h"
#include "map.h"
#include "vector.h"

void entity_demon_init(entity_t * e);
void entity_demon_update(entity_t * e);

// todo, once animations are actually parse-able
animation_t demon_animations[] = {
    {   // 0: Idle
        .time = 0.5,
        .num_frames = 6,
        .frames = (animation_frame_t[]) {
            {.name = "default"},
            {.name = "breathe.001"},
            {.name = "breathe.002"},
            {.name = "breathe.003"},
            {.name = "breathe.004"},
            {.name = "breathe.005"},
        },
    }
};

// hack for caching parsed frame names per-map
static ref_entt_t * last_ref_entt = NULL;

void entity_demon_constructor(entity_t * e) {
    entity_constructor(e);
    e->_update = entity_demon_update;
    entity_demon_init(e);
}

void entity_demon_init(entity_t * e) {

    entity_parse_animation_frames(
        e->_params->entity_generic_params.ref_entt,
        demon_animations,
        sizeof(demon_animations)/sizeof(demon_animations[0]),
        &last_ref_entt
    );

    e->_animation_collection = (animation_collection_t) {
        .animations = demon_animations,
        .num_animations = sizeof(demon_animations)/sizeof(demon_animations[0]),
    };

    e->_anim = &(e->_animation_collection.animations[0]);
    e->_anim_time = 0;

    entity_set_model(e);
}

void entity_demon_update(entity_t * e) {
    e->_draw_model(e);
}