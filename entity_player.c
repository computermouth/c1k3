
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "entity.h"
#include "entity_player.h"
#include "game.h"
#include "math.h"
#include "input.h"
#include "render.h"
#include "game.h"
#include "weapon.h"

void entity_player_init(entity_t * e, uint8_t p1, uint8_t p2);
void entity_player_update(entity_t * e);
void entity_player_receive_damage(entity_t * e, entity_t * from, int32_t amount);
void entity_player_kill(entity_t * e);

void entity_player_constructor(entity_t *e, vec3_t pos, uint8_t p1, uint8_t p2) {

    entity_constructor(e, pos, p1, p2);

    // todo, these casts kinda suck
    e->_init = (void (*)(void *, uint8_t, uint8_t))entity_player_init;
    e->_update = (void (*)(void *))entity_player_update;
    e->_receive_damage = (void (*)(void * e, void * from, int32_t amount))entity_player_receive_damage;
    e->_kill = (void (*)(void *))entity_player_kill;

    // todo, kinda goofy paradigm to set the callback, immediately invoke
    // then never call again. could just combine constructor and init I think
    e->_init(e, p1, p2);
}

void entity_player_init(entity_t * e, uint8_t p1, uint8_t p2) {

    e->s = vec3(12,24,12);
    e->f = 10;
    e->_speed = 3000;
    e->_step_height = 17;
    e->_can_jump = false;
    e->_can_shoot_at = 0.0f;
    e->_health = 100;

    e->_check_against = ENTITY_GROUP_ENEMY;

    // todo, e->_weapons = [new weapon_shotgun_t];
    e->_weapons[0] = (weapon_t) {
        0
    };
    e->_weapons[1] = (weapon_t) {
        0
    };
    e->_weapons[2] = (weapon_t) {
        0
    };
    e->_weapon_index = 0;

    // Map 1 needs some rotation of the starting look-at direction
    e->_yaw += game_map_index * PI;
    e->_bob = 0;

    game_entity_player = e;

    game_entities_friendly_push(e);
}

void entity_player_update(entity_t * e) {
    // Mouse look
    e->_pitch = clamp(
                    e->_pitch + mouse_y * mouse_speed * (mouse_invert ? -0.00015 : 0.00015),
                    -1.5f, 1.5f
                );
    e->_yaw = fmodf((e->_yaw + mouse_x * mouse_speed * 0.00015), PI * 2);

    // big todo, needs input.c
    /*
    		// Acceleration in movement direction
    		this.a = vec3_mulf(
    			vec3_rotate_y(
    				vec3(
    					keys[key_right] - keys[key_left],
    					0,
    					keys[key_up] - keys[key_down]
    				),
    				this._yaw
    			),
    			this._speed * (this._on_ground ? 1 : 0.3)
    		);

    		if (keys[key_jump] && this._on_ground && this._can_jump) {
    			this.v.y = 400;
    			this._on_ground = 0;
    			this._can_jump = 0;
    		}
    		if (!keys[key_jump]) {
    			this._can_jump = 1;
    		}

    		this._weapon_index = (
    			this._weapon_index + keys[key_next] + this._weapons.length - keys[key_prev]
    		) % this._weapons.length;

    		let shoot_wait = this._can_shoot_at - game_time,
    			weapon = this._weapons[this._weapon_index];

    		// Shoot Weapon
    		if (keys[key_action] && shoot_wait < 0) {
    			this._can_shoot_at = game_time + weapon._reload;

    			if (weapon._needs_ammo && weapon._ammo == 0) {
    				audio_play(sfx_no_ammo);
    			}
    			else {
    				weapon._shoot(this.p, this._yaw, this._pitch);
    				game_spawn(entity_light_t, this.p, 10, 0xff)._die_at = game_time + 0.1;
    			}
    		}

    		this._bob += vec3_length(this.a) * 0.0001;
    		this.f = this._on_ground ? 10 : 2.5;
    		this._update_physics();
    */
    r_camera.x = e->p.x;
    r_camera.z = e->p.z;

    // Smooth step up on stairs
    r_camera.y = e->p.y + 8 - clamp(game_time - e->_stepped_up_at, 0, 0.1) * -160;

    r_camera_yaw = e->_yaw;
    r_camera_pitch = e->_pitch;
    /*

    		// Draw weapon at camera position at an offset and add the current
    		// recoil (calculated from shoot_wait and weapon._reload) accounting
    		// for the current view yaw/pitch

    		r_draw(
    			vec3_add(
    				r_camera,
    				vec3_rotate_yaw_pitch(
    					vec3(
    						0,
    						-10 + Math.sin(this._bob)*0.3,
    						12 + clamp(scale(shoot_wait, 0, weapon._reload, 5, 0), 0, 5)
    					),
    					this._yaw, this._pitch
    				)
    			),
    			this._yaw + Math.PI/2, this._pitch,
    			weapon._texture, weapon._model.f[0], weapon._model.f[0], 0,
    			weapon._model.nv
    		);

    		h.textContent = this._health|0;
    		a.textContent = weapon._needs_ammo ? weapon._ammo : '∞';

    		// Debug: a light around the player
    		// r_push_light(vec3_add(this.p, vec3(0,64,0)), 10, 255, 192, 32);
    */
}

void entity_player_receive_damage(entity_t * e, entity_t * from, int32_t amount) {
    // todo
    // audio_play(sfx_hurt);
    // invoke regular damage thing
    entity_receive_damage(e, from, amount);
}

void entity_player_kill(entity_t * e) {
    entity_kill(e);
    // todo
    // h.textContent = this._health|0;
    title_show_message("YOU DIED", "");
    // todo
    // setTimeout(() => game_init(game_map_index), 2000);
    game_init(game_map_index);
}