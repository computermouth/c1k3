
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

animation_t default_anim[] = {
    {
        .time = 1,
        .num_frames = 1,
        .frames = (animation_frame_t[]) {
            {
                0
            }
        },
    }
};

animation_collection_t default_anim_collection = {
    .animations = default_anim,
    .num_animations = 1,
};

int64_t entity_frame_from_name(char * needle, char (*haystack)[][100], size_t len) {
    int64_t rc = -1;
    for(size_t i = 0; i < len; i++) {
        if (strcmp(needle, (*haystack)[i]) == 0)
            return i;
    }
    return rc;
}

char * entity_param_lookup(char * key, vector * v) {
    size_t plen = vector_size(v);
    for(size_t i = 0; i < plen; i++) {
        entity_extra_params_t * ep = vector_at(v, i);
        if(strcmp(key, ep->k) == 0)
            return ep->v;
    }
    fprintf(stderr, "E: failed to lookup key '%s'\n", key);
    return NULL;
}

void entity_parse_animation_frames(ref_entt_t * curr_entt, animation_t * animations, size_t anim_len, ref_entt_t ** last_entt) {
    // already cached
    if(curr_entt == *last_entt)
        return;

    for(size_t i = 0; i < anim_len; i++) {
        animation_t tmp_anim = animations[i];
        for(size_t j = 0; j < tmp_anim.num_frames; j++) {
            char * needle = tmp_anim.frames[j].name;
            int64_t f = entity_frame_from_name(needle, curr_entt->frame_names, curr_entt->frame_len);
            if(f < 0) {
                fprintf(stderr, "E: couldn't find frame %s for %s\n", needle, curr_entt->entity_name);
                continue;
            }
            tmp_anim.frames[j].id = f;
        }
    }

    *last_entt = curr_entt;
}

void entity_set_model(entity_t * e) {
    e->_texture = e->_params->entity_generic_params.ref_entt->tex_id;
    vector * frames = e->_params->entity_generic_params.ref_entt->frames;
    uint32_t * uframes = vector_begin(frames);
    e->_model.frames = uframes;
    e->_model.nv = e->_params->entity_generic_params.ref_entt->vert_len;
    e->s = e->_params->entity_generic_params.ref_entt->size;
}

void entity_constructor(entity_t *e) {

    e->p = e->_params->position;
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
    e->_init = entity_init;
    e->_update = entity_update;
    e->_update_physics = entity_update_physics;
    e->_collides = entity_collides;
    e->_did_collide = entity_did_collide;
    e->_did_collide_with_entity = entity_did_collide_with_entity;
    e->_draw_model = entity_draw_model;
    e->_spawn_particles = entity_spawn_particles;
    e->_receive_damage = entity_receive_damage;
    e->_play_sound = entity_play_sound;
    e->_kill = entity_kill;
    e->_pickup = entity_pickup;
    e->_set_state = entity_set_state;
    e->_spawn_projectile = entity_spawn_projectile;
    e->_attack = entity_attack;

    entity_init(e);
}

// only to do something dynamic to every entity
void entity_init(entity_t * e) {
    e = e;
}

void entity_update(entity_t * e) {
    e->_draw_model(e);
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

void entity_did_collide(entity_t * e, int axis) {
    e = e;
    axis = axis;
}

void entity_did_collide_with_entity(entity_t * e, entity_t * other) {
    e = e;
    other = other;
}

void entity_draw_model(entity_t * e) {
    e->_anim_time += game_tick;

    // Calculate which frames to use and how to mix them
    float f = e->_anim_time / (float)e->_anim->time;
    float mix = f - floorf(f);

    uint32_t frame_cur = 0;
    uint32_t frame_next = 0;

    frame_cur = e->_anim->frames[(uint32_t)f % e->_anim->num_frames].id;
    frame_next = e->_anim->frames[(1 + (uint32_t)f) % e->_anim->num_frames].id;

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
        .f1 = e->_model.frames[frame_cur],
        .f2 = e->_model.frames[frame_next],
        .mix = mix,
        .num_verts = e->_model.nv
    };
    r_draw(call);
}

void entity_spawn_particles(entity_t * e, uint32_t amount, float speed, entity_id_t eid, float lifetime) {

    entity_params_t ep = map_entt_params_from_eid(eid);

    // scooch back 1/16th, so the particles aren't
    // just stuck in a wall
    vec3_t move_dist = vec3_mulf(e->v, game_tick);
    vec3_t tickdist = vec3_divf(move_dist, 16.0f);

    ep.position = vec3_sub(e->p, tickdist);

    for (uint32_t i = 0; i < amount; i++) {
        entity_t * particle = game_spawn(&ep);
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
    // silence Wunused
    from = from;

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

void entity_pickup(entity_t * e) {
    e = e;
}
void entity_set_state(entity_t * e, uint32_t state) {
    e = e;
    state = state;
}
void entity_attack(entity_t * e) {
    e = e;
}
entity_t * entity_spawn_projectile(entity_t * e, entity_id_t eid, float speed, float yaw_offset, float pitch_offset) {
    // silence Wunused
    e = e;
    eid = eid;
    speed = speed;
    yaw_offset = yaw_offset;
    pitch_offset = pitch_offset;

    return NULL;
}
