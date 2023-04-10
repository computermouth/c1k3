

#include "entity.h"

uint32_t no_idea_placeholder[] = {0};

void entity_constructor(entity_t * e, vec3_t pos, vec3_t p1, vec3_t p2) {

    entity_t tmp = {0};

    tmp.p = pos;
    tmp.s = (vec3_t) {
        2.0f, 2.0f, 2.0f
    };

    tmp._health = 50;
    tmp._gravity = 1;
    tmp._anim = (animation_t) {
        .length = 0, .no_idea = no_idea_placeholder
    };
    tmp._anim_time = randf();

    *e = tmp;
    entity_init(e, p1, p2);
}

void entity_init(entity_t * e, vec3_t p1, vec3_t p2) {}

// void entity_update(entity_t * e){
//     if (e->_model) {
//         entity_draw_model(e);
//     }
// }

