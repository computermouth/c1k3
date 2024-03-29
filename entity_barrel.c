
#include <stdint.h>
#include <stdlib.h>

#include "entity.h"
#include "entity_light.h"
#include "entity_barrel.h"
#include "game.h"
#include "model.h"
#include "audio.h"

void entity_barrel_init(entity_t * e);
void entity_barrel_kill(entity_t * e);
void entity_barrel_update(entity_t * e);

void entity_barrel_constructor(entity_t *e) {
    entity_constructor(e);

    e->_kill = entity_barrel_kill;
    e->_update = entity_barrel_update;

    entity_barrel_init(e);
}

void entity_barrel_update(entity_t * e) {
    e->_update_physics(e);
    e->_draw_model(e);
}

void entity_barrel_init(entity_t * e) {
    e->_health = 10;
    e->_group = ENTITY_GROUP_ENEMY;

    entity_set_model(e);
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

    // todo, barrelgib
    for (uint32_t i = ENTITY_ID_GIBS000; i <= ENTITY_ID_GIBS006; i++) {
        e->_spawn_particles(e, 2, 600, i, 1);
    }

    entity_params_t l = {
        .id = ENTITY_ID_LIGHT,
        .position = vec3_add(e->p, vec3(0,16,0)),
        .entity_light_params = {
            .rgba[0] = 0xE0,
            .rgba[1] = 0x60,
            .rgba[2] = 0x80,
            .rgba[3] = 0xFF,
        },
    };
    entity_t * tmp_light = game_spawn(&l);

    tmp_light->_expires = true;
    tmp_light->_die_at = game_time + 0.2;

    game_entities_enemies_pop(&e);
}