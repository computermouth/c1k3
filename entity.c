
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "game.h"
#include "entity.h"
#include "math.h"
#include "render.h"
#include "map.h"
#include "entity_particle.h"

uint32_t no_idea_placeholder[] = {0};

void entity_constructor(entity_t *e, vec3_t pos, uint8_t p1, uint8_t p2) {

    e->p = pos;
    e->s = (vec3_t) {
        2.0f, 2.0f, 2.0f
    };

    e->_health = 50;
    e->_gravity = 1;
    // todo, maybe??? e->_anim
    e->_anim_time = randf();

    // todo, rename, f_init?
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_init;
    e->_update = (void (*)(void *))entity_update;
    e->_update_physics = (void (*)(void *))entity_update_physics;
    e->_collides = (bool (*)(void * e, vec3_t p))entity_collides;
    e->_did_collide = (void (*)(int axis))entity_did_collide;
    e->_did_collide_with_entity = (void (*)(void * e, void * other))entity_did_collide_with_entity;
    e->_draw_model = (void (*)(void * e))entity_draw_model;
    e->_spawn_particles = (void (*)(void * e, int amount, int speed, model_t * model, int texture, float lifetime))entity_spawn_particles;
    e->_receive_damage = (void (*)(void * e, void * from, int32_t amount))entity_receive_damage;
    e->_play_sound = (void (*)(void * e, void * sound))entity_play_sound;
    e->_kill = (void (*)(void * e))entity_kill;

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
    if (e->_expires && e->_die_at < game_tick) {
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
    switch(e->_check_against) {
    case ENTITY_GROUP_NONE:
        e->_check_entities = NULL;
        break;
    case ENTITY_GROUP_PLAYER:
        e->_check_entities = &game_entities_friendly;
    case ENTITY_GROUP_ENEMY:
        e->_check_entities = &game_entities_enemies;
        break;
    }

    // Divide the physics integration into 16 unit steps; otherwise fast
    // projectiles may just move through walls.
    int32_t original_step_height = e->_step_height;
    vec3_t move_dist = vec3_mulf(e->v, game_tick);
    int32_t steps = ceilf(vec3_length(move_dist) / 16.0f);
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
                e->_did_collide(0);
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
                e->_did_collide(2);
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
            e->_did_collide(1);
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

    entity_collection_t * check = e->_check_entities;
    for (uint32_t i = 0; check !=NULL && i < check->length; i++) {
        entity_t * chk_e = &(check->entities[i]);
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
        !map_block_at( (int32_t)(p.x) >> 5, (int32_t)(p.y - e->s.y - 8) >> 4, (int32_t)(p.z) >> 5 ) &&
        !map_block_at( (int32_t)(p.x) >> 5, (int32_t)(p.y - e->s.y - 24) >> 4, (int32_t)(p.z) >> 5)
    ) {
        return true;
    }

    // Do the normal collision check with the whole box
    return map_block_at_box(vec3_sub(p, e->s), vec3_add(p, e->s));
}

void entity_did_collide(int axis) {}

void entity_did_collide_with_entity(entity_t * e, entity_t * other) {}

void entity_draw_model(entity_t * e) {
    e->_anim_time += game_tick;
    // Calculate which frames to use and how to mix them
    float f = e->_anim_time / (float)e->_anim.time;
    float mix = f - floorf(f);
    uint32_t frame_cur = e->_anim.frames[(int32_t)f % e->_anim.num_frames];
    uint32_t frame_next = e->_anim.frames[(1 + (int32_t)f) % e->_anim.num_frames];

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

void entity_play_sound(entity_t * e, void * sound) {
    float volume = clamp(scale(vec3_dist(e->p, r_camera), 64, 1200, 1, 0),0,1);
    float pan = sinf(vec3_2d_angle(e->p, r_camera)-r_camera_yaw)*-1;
    // todo -- remove and actually play sound
    volume = pan;
    pan = volume;
    // audio_play(sound, volume, 0, pan);
}

void entity_kill(entity_t * e) {
    e->_dead = 1;
}
