
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "game.h"
#include "entity.h"
#include "math.h"
#include "render.h"
#include "map.h"
#include "entity_particle.h"
#include "audio.h"

uint32_t no_idea_placeholder[] = {0};

animation_t default_anim[] = {
    {
        .frames = (uint32_t[]){ 0 },
        .num_frames = 1,
        .time = 1,
    }
};

animation_collection_t default_anim_collection = {
    .animations = default_anim,
    .num_animations = 1,
};

void entity_constructor(entity_t *e, vec3_t pos, uint8_t p1, uint8_t p2) {

    e->p = pos;
    e->s = (vec3_t) {
        2.0f, 2.0f, 2.0f
    };

    e->_health = 50;
    e->_gravity = 1;
    // todo, maybe??? e->_anim
    e->_animation_collection = default_anim_collection;
    e->_anim = &(default_anim_collection.animations[0]);
    e->_anim_time = randf();

    // todo, I hate this
    e->_STATE_IDLE = -1;
    e->_STATE_PATROL = -1;
    e->_STATE_FOLLOW = -1;
    e->_STATE_ATTACK_RECOVER = -1;
    e->_STATE_ATTACK_EXEC = -1;
    e->_STATE_ATTACK_PREPARE = -1;
    e->_STATE_ATTACK_AIM = -1;
    e->_STATE_EVADE = -1;

    // todo, rename, f_init?
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_init;
    e->_update = (void (*)(void *))entity_update;
    e->_update_physics = (void (*)(void *))entity_update_physics;
    e->_collides = (bool (*)(void * e, vec3_t p))entity_collides;
    e->_did_collide = (void (*)(void * e, int axis))entity_did_collide;
    e->_did_collide_with_entity = (void (*)(void * e, void * other))entity_did_collide_with_entity;
    e->_draw_model = (void (*)(void * e))entity_draw_model;
    e->_spawn_particles = (void (*)(void * e, int amount, int speed, model_t * model, int texture, float lifetime))entity_spawn_particles;
    e->_receive_damage = (void (*)(void * e, void * from, int32_t amount))entity_receive_damage;
    e->_play_sound = (void (*)(void * e, Mix_Chunk * sound))entity_play_sound;
    e->_kill = (void (*)(void * e))entity_kill;
    e->_pickup = (void (*)(void * e))entity_pickup;
    e->_set_state = (void (*)(void * e, uint32_t state))entity_set_state;
    e->_spawn_projectile = (void * (*)(void * e, void (*)(void *, vec3_t, uint8_t, uint8_t), float speed, float yaw_offset, float pitch_offset))entity_pickup;
    e->_attack = (void (*)(void * e))entity_attack;

    entity_init(e, 0, 0);
}

// only to do something dynamic to every entity
void entity_init(entity_t * e, uint8_t p1, uint8_t p2) {}

void entity_update(entity_t * e) {
    if (e->_model) {
        e->_draw_model(e);
    }
}

void entity_update_physics(entity_t * e) {
    // todo, move to entity_update?
    if (e->_expires && e->_die_at < game_time) {
        e->_kill(e);
    }

    // Apply Gravity
    e->a.y = -1200 * e->_gravity;

    // Integrate acceleration & friction into velocity
    float ff = min(e->f * game_tick, 1);
    e->v = vec3_add(e->v,
                    vec3_sub(
                        vec3_mulf(e->a, game_tick),
                        vec3_mul(e->v, vec3(ff, 0.0f, ff))
                    )
                   );

    // Set up the _check_entities array for entity collisions
    // todo, maybe allow falling through, append pointer to entities
    // _check_entities.push(ptr_to_enemies)
    // _check_entities.push(ptr_to_friendlies)
    // todo, also move to _init or something, why is this here???
    // actually needs to happen currently, because these flags aren't
    // set before the generic init()
    switch(e->_check_against) {
    case ENTITY_GROUP_NONE:
        e->_check_entities = NULL;
        break;
    case ENTITY_GROUP_PLAYER:
        e->_check_entities = game_entities_list_friendly;
        break;
    case ENTITY_GROUP_ENEMY:
        e->_check_entities = game_entities_list_enemies;
        break;
    }

    // Divide the physics integration into 16 unit steps; otherwise fast
    // projectiles may just move through walls.
    int32_t original_step_height = e->_step_height;
    vec3_t move_dist = vec3_mulf(e->v, game_tick);
    float steps = ceilf(vec3_length(move_dist) / 16.0f);
    vec3_t move_step = vec3_mulf(move_dist, 1.0f/(float)steps);

    for (int s = 0; s < steps; s++) {
        // Remember last position so we can roll back
        vec3_t lp = vec3_clone(e->p);

        // Integrate velocity into position
        e->p = vec3_add(e->p, move_step);

        // Collision with walls, horizonal
        if(e->_collides(e, vec3(e->p.x, lp.y, lp.z))) {
            // Can we step up?
            if (
                !e->_step_height || !e->_on_ground || e->v.y > 0 ||
                e->_collides(e, vec3(e->p.x, lp.y + e->_step_height, lp.z))
            ) {
                e->_did_collide(e, 0);
                e->p.x = lp.x;
                e->v.x = -(e->v.x) * e->_bounciness;
            } else {
                lp.y += e->_step_height;
                e->_stepped_up_at = game_time;
            }
            s = steps; // stop after this iteration
        }

        // Collision with walls, vertical
        if (e->_collides(e, vec3(e->p.x, lp.y, e->p.z))) {

            // Can we step up?
            if (
                !e->_step_height || !e->_on_ground || e->v.y > 0 ||
                e->_collides(e, vec3(e->p.x, lp.y+e->_step_height, e->p.z))
            ) {
                e->_did_collide(e, 2);
                e->p.z = lp.z;
                e->v.z = -(e->v.z) * e->_bounciness;
            } else {
                lp.y += e->_step_height;
                e->_stepped_up_at = game_time;
            }

            s = steps; // stop after this iteration
        }


        // Collision with ground/Ceiling
        if (e->_collides(e, e->p)) {
            e->_did_collide(e, 1);
            e->p.y = lp.y;

            // Only bounce from ground/ceiling if we have enough velocity
            float bounce = fabs(e->v.y) > 200 ? e->_bounciness : 0;
            e->_on_ground = e->v.y < 0 && !bounce;
            e->v.y = -(e->v).y * bounce;

            s = steps; // stop after this iteration
        }

        e->_step_height = original_step_height;
    }
}

bool entity_collides(entity_t * e, vec3_t p) {

    if (e->_dead)
        return false;

    vector * check = e->_check_entities;
    uint32_t len = 0;
    // skips loop if check is null
    if (check != NULL)
        len = vector_size(check);
    for (uint32_t i = 0; i < len; i++) {
        entity_t ** chk_p = vector_at(check, i);
        entity_t *  chk_e = *chk_p;
        if (vec3_dist(p, chk_e->p) < e->s.y + chk_e->s.y) {
            // If we collide with an entity set the step height to 0,
            // so we don't climb up on its shoulders :/
            e->_step_height = 0;
            e->_did_collide_with_entity(e, chk_e);
            return true;
        }
    }

    // Check if there's no block beneath this point. We want the AI to keep
    // off of ledges.
    if (
        e->_on_ground && e->_keep_off_ledges &&
        !map_block_at( (uint32_t)(p.x) >> 5, (uint32_t)(p.y - e->s.y - 8) >> 4, (uint32_t)(p.z) >> 5 ) &&
        !map_block_at( (uint32_t)(p.x) >> 5, (uint32_t)(p.y - e->s.y - 24) >> 4, (uint32_t)(p.z) >> 5)
    ) {
        return true;
    }

    // Do the normal collision check with the whole box
    return map_block_at_box(vec3_sub(p, e->s), vec3_add(p, e->s));
}

void entity_did_collide(entity_t * e, int axis) {}

void entity_did_collide_with_entity(entity_t * e, entity_t * other) {}

void entity_draw_model(entity_t * e) {
    e->_anim_time += game_tick;

    // Calculate which frames to use and how to mix them
    float f = e->_anim_time / (float)e->_anim->time;
    float mix = f - floorf(f);
    uint32_t frame_cur = e->_anim->frames[(int32_t)f % e->_anim->num_frames];
    uint32_t frame_next = e->_anim->frames[(1 + (int32_t)f) % e->_anim->num_frames];

    // Swap frames if we're looping to the first frame again
    if (frame_next < frame_cur) {
        uint32_t tmp = frame_cur;
        frame_cur = frame_next;
        frame_next = tmp;
        mix = 1 - mix;
    }
    draw_call_t call = {
        .pos = e->p,
        .yaw = e->_yaw,
        .pitch = e->_pitch,
        .texture = e->_texture,
        .f1 = e->_model->frames[frame_cur],
        .f2 = e->_model->frames[frame_next],
        .mix = mix,
        .num_verts = e->_model->nv
    };
    r_draw(call);
}

void entity_spawn_particles(entity_t * e, int amount, int speed, model_t * model, int texture, float lifetime) {
    for (uint32_t i = 0; i < amount; i++) {
        entity_t * particle = game_spawn((void (*)(entity_t *, vec3_t, uint8_t, uint8_t))entity_particle_constructor, e->p, 0, 0);
        particle->_model = model;
        particle->_texture = texture;
        particle->_expires = true;
        particle->_die_at = game_time + lifetime + randf() * lifetime * 0.2;
        particle->v = vec3(
                          (randf() - 0.5) * speed,
                          randf() * speed,
                          (randf() - 0.5) * speed
                      );
    }
}

void entity_receive_damage(entity_t * e, entity_t * from, int32_t amount) {
    if (e->_dead)
        return;

    e->_health -= amount;
    if (e->_health <= 0)
        e->_kill(e);
}

void entity_play_sound(entity_t * e, Mix_Chunk * sound) {
    float volume = clamp(scale(vec3_dist(e->p, r_camera), 64, 1200, 1, 0),0,1);
    float pan = sinf(vec3_2d_angle(e->p, r_camera)-r_camera_yaw)*-1;
    audio_play_opt(sound, volume, 0, pan);
}

void entity_kill(entity_t * e) {
    e->_dead = 1;
}

void entity_pickup(entity_t * e) {}
void entity_set_state(entity_t * e, uint32_t state) {}
void entity_attack(entity_t * e) {}
entity_t * entity_spawn_projectile(entity_t * e, void (*func)(entity_t *, vec3_t, uint8_t, uint8_t), float speed, float yaw_offset, float pitch_offset) {
    return NULL;
}
