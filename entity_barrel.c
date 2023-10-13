
#include <stdint.h>
#include <stdlib.h>

#include "entity.h"
#include "entity_light.h"
#include "entity_barrel.h"
#include "game.h"
#include "model.h"
#include "audio.h"

void entity_barrel_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_barrel_kill(entity_t * e);

void entity_barrel_constructor(entity_t *e, vec3_t pos, uint8_t p1, uint8_t p2) {
    entity_constructor(e, pos, p1, p2);

    e->_init = entity_barrel_init;
    e->_kill = (void (*)(void *))entity_barrel_kill;

    e->_init(e, p1, p2);
}

void entity_barrel_init(entity_t * e, uint8_t p1, uint8_t p2) {
    e->_model = &model_barrel;
    e->_texture = 21;
    e->_pitch = PI/2.0f;
    e->_health = 10;
    e->s = vec3(8, 32, 8);
    e->_group = ENTITY_GROUP_ENEMY;
}

void entity_barrel_kill(entity_t * e) {
    // Deal some damage to nearby entities
    uint32_t len = vector_size(game_entities_list_enemies);
    for (uint32_t i = 0; i < len; i++) {
        entity_t ** entity_p = vector_at(game_entities_list_enemies, i);
        entity_t * entity = *entity_p;
        float dist = vec3_dist(e->p, entity->p);
        if (entity != e && dist < 256) {
            entity->_receive_damage(entity, e, scale(dist, 0, 256, 60, 0));
        }
    }

    entity_kill(e);
    e->_play_sound(e, sfx_grenade_explode);

    len = vector_size(model_gib_pieces);
    for (uint32_t i = 0; i < len; i++) {
        model_t * m = vector_at(model_gib_pieces, i);
        e->_spawn_particles(e, 2, 600, m, 21, 1);
    }
    entity_t * tmp_light = game_spawn(entity_light_constructor, vec3_add(e->p, vec3(0,16,0)), 250, 0x08f, NULL);
    tmp_light->_expires = true;
    tmp_light->_die_at = game_time + 0.2;

    game_entities_enemies_pop(&e);
}