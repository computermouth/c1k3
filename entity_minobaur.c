
#include "entity.h"
#include "game.h"
#include "audio.h"
#include "map.h"
#include "vector.h"

void entity_minobaur_init(entity_t * e);
void entity_minobaur_update(entity_t * e);

animation_t minobaur_animations[] = {
    {   // 0: T pose
        .time = 0.5,
        .num_frames = 1,
        .frames = (animation_frame_t[]) {
            {.name = "default"},
        },
    },
    {   // 1: Swipe
        .time = 0.09,
        .num_frames = 20,
        .frames = (animation_frame_t[]) {
            {.name = "swipe.000"},
            {.name = "swipe.001"},
            {.name = "swipe.002"},
            {.name = "swipe.003"},
            {.name = "swipe.004"},
            {.name = "swipe.005"},
            {.name = "swipe.006"},
            {.name = "swipe.007"},
            {.name = "swipe.008"},
            {.name = "swipe.009"},
            {.name = "swipe.010"},
            {.name = "swipe.011"},
            {.name = "swipe.012"},
            {.name = "swipe.013"},
            {.name = "swipe.014"},
            {.name = "swipe.015"},
            {.name = "swipe.016"},
            {.name = "swipe.017"},
            {.name = "swipe.018"},
            {.name = "swipe.019"},
        },
    },
    {   // 2: Charge
        .time = 0.10,
        .num_frames = 5,
        .frames = (animation_frame_t[]) {
            {.name = "charge.000"},
            {.name = "charge.001"},
            {.name = "charge.002"},
            {.name = "charge.003"},
            {.name = "charge.004"},
        },
    },
    {   // 3: Land
        .time = .50,
        .num_frames = 7,
        .frames = (animation_frame_t[]) {
            {.name = "land.000"},
            {.name = "land.000"},
            {.name = "land.000"},
            {.name = "land.000"},
            {.name = "land.001"},
            {.name = "land.002"},
            {.name = "land.003"},
            {.name = "land.004"},
            {.name = "swipe.000"},
        },
    },
    {   // 4: Die
        .time = 0.15,
        .num_frames = 20,
        .frames = (animation_frame_t[]) {
            {.name = "die.000"},
            {.name = "die.001"},
            {.name = "die.002"},
            {.name = "die.003"},
            {.name = "die.004"},
            {.name = "die.005"},
            {.name = "die.006"},
            {.name = "die.007"},
            {.name = "die.008"},
            {.name = "die.009"},
            {.name = "die.010"},
            {.name = "die.011"},
            {.name = "die.012"},
            {.name = "die.013"},
            {.name = "die.013"},
            {.name = "die.013"},
            {.name = "die.013"},
            {.name = "die.013"},
            {.name = "die.013"},
            {.name = "die.013"},
        },
    },
    {   // 5: Bump
        .time = 0.12,
        .num_frames = 9,
        .frames = (animation_frame_t[]) {
            {.name = "bump.000"},
            {.name = "bump.001"},
            {.name = "bump.002"},
            {.name = "bump.003"},
            {.name = "bump.004"},
            {.name = "bump.005"},
            {.name = "bump.006"},
            {.name = "bump.007"},
            {.name = "bump.008"},
        },
    },
};

// hack for caching parsed frame names per-map
static ref_entt_t * last_ref_entt = NULL;

void entity_minobaur_constructor(entity_t * e) {
    entity_constructor(e);
    e->_update = entity_minobaur_update;
    entity_minobaur_init(e);
}

void entity_minobaur_init(entity_t * e) {

    entity_parse_animation_frames(
        e->_params->entity_generic_params.ref_entt,
        minobaur_animations,
        sizeof(minobaur_animations)/sizeof(minobaur_animations[0]),
        &last_ref_entt
    );

    e->_animation_collection = (animation_collection_t) {
        .animations = minobaur_animations,
        .num_animations = sizeof(minobaur_animations)/sizeof(minobaur_animations[0]),
    };

    e->_anim = &(e->_animation_collection.animations[1]);
    e->_anim_time = 0;

    entity_set_model(e);
}

void entity_minobaur_update(entity_t * e) {
    e->_draw_model(e);
}