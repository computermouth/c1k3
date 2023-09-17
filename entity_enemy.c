
#include <stdlib.h>
#include <math.h>

#include "entity.h"
#include "entity_enemy.h"
#include "game.h"
#include "math.h"
#include "model.h"
#include "map.h"
#include "audio.h"

// todo, all this state stuff is pretty bad
// I don't think it'll hold up well with the fake OOP

animation_t default_animations[] = {
    {   // 0: Idle
        .time = 1,
        .num_frames = 1,
        .frames = (uint32_t[]){0},
    },
    {   // 1: Walk
        .time = 0.40f,
        .num_frames = 4,
        .frames = (uint32_t[]){1,2,3,4},
    },
    {   // 2: Run
        .time = 0.20f,
        .num_frames = 4,
        .frames = (uint32_t[]){1,2,3,4},
    },
    {   // 3: Attack prepare
        .time = 0.25f,
        .num_frames = 4,
        .frames = (uint32_t[]){0,5,5,5},
    },
    {   // 4: Attack
        .time = 0.25f,
        .num_frames = 4,
        .frames = (uint32_t[]){5,0,0,0},
    },
};

enemy_state_t default_enemy_states[_ENEMY_STATE_NULL] = {
    {ENEMY_ANIMATION_IDLE,   0, 0.1, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_WALK, 0.5, 0.5, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_RUN,   1, 0.3, _ENEMY_STATE_NULL},
    {ENEMY_ANIMATION_IDLE,   0, 0.1, ENEMY_STATE_FOLLOW},
    {ENEMY_ANIMATION_ATTACK,   0, 0.4, ENEMY_STATE_ATTACK_RECOVER},
    {ENEMY_ANIMATION_ATTACK_PREPARE,   0, 0.4, ENEMY_STATE_ATTACK_EXEC},
    {ENEMY_ANIMATION_IDLE,   0, 0.1, ENEMY_STATE_ATTACK_PREPARE},
    {ENEMY_ANIMATION_RUN,   1, 0.8, ENEMY_STATE_ATTACK_AIM},
};

void entity_enemy_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_enemy_set_state(entity_t * e, uint32_t state);
void entity_enemy_update(entity_t * e);
entity_t * entity_enemy_spawn_projectile(entity_t * e, void (*func)(entity_t *, vec3_t, uint8_t, uint8_t, entity_params_t *), float speed, float yaw_offset, float pitch_offset);
void entity_enemy_receive_damage(entity_t * e, entity_t * from, int32_t amount);
void entity_enemy_kill(entity_t * e);
void entity_enemy_did_collide(entity_t * e, int axis);

void entity_enemy_constructor(entity_t *e, vec3_t pos, uint8_t p1, uint8_t p2, entity_params_t * ep) {

    entity_constructor(e, pos, p1, p2, ep);

    // todo, still hate this
    e->_STATE_IDLE              = ENEMY_STATE_IDLE;
    e->_STATE_PATROL            = ENEMY_STATE_PATROL;
    e->_STATE_FOLLOW            = ENEMY_STATE_FOLLOW;
    e->_STATE_ATTACK_RECOVER    = ENEMY_STATE_ATTACK_RECOVER;
    e->_STATE_ATTACK_EXEC       = ENEMY_STATE_ATTACK_EXEC;
    e->_STATE_ATTACK_PREPARE    = ENEMY_STATE_ATTACK_PREPARE;
    e->_STATE_ATTACK_AIM        = ENEMY_STATE_ATTACK_AIM;
    e->_STATE_EVADE             = ENEMY_STATE_EVADE;
    e->_STATE_NULL              = _ENEMY_STATE_NULL;

    // todo, these casts kinda suck
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_enemy_init;
    e->_set_state = (void (*)(void * e, uint32_t state))entity_enemy_set_state;
    e->_update = (void (*)(void *))entity_enemy_update;
    e->_spawn_projectile = (void * (*)(void * e, void (*)(void *, vec3_t, uint8_t, uint8_t), float speed, float yaw_offset, float pitch_offset))entity_enemy_spawn_projectile;
    e->_receive_damage = (void (*)(void * e, void * from, int32_t amount))entity_enemy_receive_damage;
    e->_kill = (void (*)(void *))entity_enemy_kill;
    e->_did_collide = (void (*)(void * e, int axis))entity_enemy_did_collide;

    // todo, kinda goofy paradigm to set the callback, immediately invoke
    // then never call again. could just combine constructor and init I think
    e->_init(e, p1, p2);
}

void entity_enemy_init(entity_t * e, uint8_t patrol_dir, uint8_t p2) {
    // todo, check patrol_dir

    e->_state_collection = (enemy_state_collection_t) {
        .num_states = _ENEMY_STATE_NULL,
        .states = default_enemy_states
    };

    e->_animation_collection = (animation_collection_t) {
        .animations = default_animations,
        .num_animations = sizeof(default_animations)/sizeof(default_animations[0]),
    };

    e->s = vec3(12,28,12);

    e->_step_height = 17;
    e->_speed = 196;

    e->_target_yaw = e->_yaw;
    e->_state_update_at = 0;
    e->_attack_distance = 800;
    e->_evade_distance = 96;
    e->_attack_chance = 0.65;
    e->_keep_off_ledges = 1;
    e->_turn_bias = 1;

    e->_group = ENTITY_GROUP_ENEMY;
    e->_check_against = ENTITY_GROUP_PLAYER;

    // If patrol_dir is non-zero it determines the partrol direction in
    // increments of 90°. Otherwise we just idle.
    if (patrol_dir) {
        e->_set_state(e, e->_STATE_PATROL);
        e->_target_yaw = (PI/2.0f) * patrol_dir;
        e->_anim_time = randf();
    }
    else {
        e->_set_state(e, e->_STATE_IDLE);
    }
}

void entity_enemy_set_state(entity_t * e, enemy_state_enum_t state) {
    e->_state = state;
    enemy_animation_enum_t index = e->_state_collection.states[state].index;
    e->_anim = &(e->_animation_collection.animations[index]);
    e->_anim_time = 0;
    float next_update = e->_state_collection.states[state].next_state_update;
    e->_state_update_at = game_time + next_update + next_update/4 * randf();
}

void entity_enemy_update(entity_t * e) {
    // Is it time for a state update?
    if (e->_state_update_at < game_time) {

        // Choose a new turning bias for FOLLOW/EVADE when we hit a wall
        e->_turn_bias = randf() > 0.5 ? 0.5 : -0.5;

        // don't if player is dead
        float distance_to_player = (game_entity_player == NULL) ? 0.0f : vec3_dist(e->p, game_entity_player->p);
        float angle_to_player = (game_entity_player == NULL) ? 0.0f : vec3_2d_angle(e->p, game_entity_player->p);

        // if there is a next_state, set it
        if (e->_state_collection.states[e->_state].next_state != e->_STATE_NULL) {
            e->_set_state(e, e->_state_collection.states[e->_state].next_state);
        }

        // Try to minimize distance to the player
        if (e->_state == e->_STATE_FOLLOW) {

            // Do we have a line of sight?
            if (game_entity_player && !map_trace(e->p, game_entity_player->p)) {
                e->_target_yaw = angle_to_player;
            }

            // Are we close enough to attack?
            if (distance_to_player < e->_attack_distance) {

                // Are we too close? Evade!
                if (
                    distance_to_player < e->_evade_distance ||
                    randf() > e->_attack_chance
                ) {
                    e->_set_state(e, ENEMY_STATE_EVADE);
                    e->_target_yaw += PI/2.0f + randf() * PI;
                }

                // Just the right distance to attack!
                else {
                    e->_set_state(e, ENEMY_STATE_ATTACK_AIM);
                }
            }
        }

        // We just attacked; just keep looking at the player 0_o
        if (e->_state == ENEMY_STATE_ATTACK_RECOVER) {
            e->_target_yaw = angle_to_player;
        }

        // Wake up from patroling or idlyng if we have a line of sight
        // and are near enough
        if (e->_state == ENEMY_STATE_PATROL || e->_state == ENEMY_STATE_IDLE) {
            if (
                distance_to_player < 700 &&
                game_entity_player &&
                !map_trace(e->p, game_entity_player->p)
            ) {
                e->_set_state(e, ENEMY_STATE_ATTACK_AIM);
            }
        }

        // Aiming - reorient the entity towards the player, check
        // if we have a line of sight
        if (e->_state == ENEMY_STATE_ATTACK_AIM) {
            e->_target_yaw = angle_to_player;

            // No line of sight? Randomly shuffle around :/
            if (game_entity_player && map_trace(e->p, game_entity_player->p)) {
                e->_set_state(e, ENEMY_STATE_EVADE);
            }
        }

        // Execute the attack!
        if (e->_state == ENEMY_STATE_ATTACK_EXEC) {
            e->_attack(e);
        }
    }

    // Rotate to desired angle
    e->_yaw += anglemod(e->_target_yaw - e->_yaw) * 0.1;


    // Move along the yaw direction with the current speed (which might be 0)
    if (e->_on_ground) {
        e->v = vec3_rotate_y(vec3(0, e->v.y, e->_state_collection.states[e->_state].speed * e->_speed), e->_target_yaw);
    }

    e->_update_physics(e);
    e->_draw_model(e);
}

entity_t * entity_enemy_spawn_projectile(entity_t * e, void (*func)(entity_t *, vec3_t, uint8_t, uint8_t, entity_params_t *), float speed, float yaw_offset, float pitch_offset) {
    entity_t * projectile = game_spawn(func, e->p, 0, 0, NULL);
    projectile->_check_against = ENTITY_GROUP_PLAYER;
    projectile->_yaw = e->_yaw + PI/2.0f;

    // shitty hack for dead player
    float pitch = atan2f(e->p.y, e->p.x) + pitch_offset;
    if (game_entity_player)
        pitch = atan2f(e->p.y - game_entity_player->p.y, vec3_dist(e->p, game_entity_player->p)) + pitch_offset;

    projectile->v = vec3_rotate_yaw_pitch(
                        vec3(0, 0, speed),
                        e->_yaw + yaw_offset,
                        pitch
                    );
    return projectile;
}

void entity_enemy_receive_damage(entity_t * e, entity_t * from, int32_t amount) {
    entity_receive_damage(e, from, amount);
    e->_play_sound(e, sfx_enemy_hit);

    // Wake up if we're idle or patrolling
    if (e->_state == ENEMY_STATE_IDLE || e->_state == ENEMY_STATE_PATROL) {
        e->_target_yaw = vec3_2d_angle(e->p, game_entity_player->p);
        e->_set_state(e, ENEMY_STATE_FOLLOW);
    }

    e->_spawn_particles(e, 2, 200, &(model_blood), 18, 0.5);
}

void entity_enemy_kill(entity_t * e) {
    entity_kill(e);

    uint32_t len = vector_size(model_gib_pieces);
    for (uint32_t i = 0; i < len; i++) {
        model_t * m = vector_at(model_gib_pieces, i);
        e->_spawn_particles(e, 2, 300, m, 18, 1);
    }
    e->_play_sound(e, sfx_enemy_gib);
    game_entities_enemies_pop(&e);
}

void entity_enemy_did_collide(entity_t * e, int axis) {
    if (axis == 1) {
        return;
    }

    // If we hit a wall/ledge while patrolling just turn around 180
    if (e->_state == ENEMY_STATE_PATROL) {
        e->_target_yaw += PI;
    }
    else {
        e->_target_yaw += e->_turn_bias;
    }
}