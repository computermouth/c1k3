
#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>

#include "math.h"
#include "vector.h"

typedef struct {
    uint8_t * data;
    uint32_t data_len;
    uint32_t * frames;
    uint32_t frame_len;
    uint32_t nv;
} model_t;

extern vector * model_data;
extern model_t model_q;
// Particles
extern model_t model_explosion;
extern model_t model_blood;
extern model_t model_gib;
extern vector * model_gib_pieces;
// Enemies
extern model_t model_grunt;
extern model_t model_enforcer;
extern model_t model_ogre;
extern model_t model_zombie;
extern model_t model_hound;
// Map Objects
extern model_t model_barrel;
extern model_t model_torch;
// Weapon view models
extern model_t model_shotgun;
extern model_t model_nailgun;
extern model_t model_grenadelauncher;
// Pickups
extern model_t model_pickup_nailgun;
extern model_t model_pickup_grenadelauncher;
extern model_t model_pickup_box;
extern model_t model_pickup_grenades;
extern model_t model_pickup_key;
extern model_t model_door;
// Projectiles
extern model_t model_grenade;
extern model_t model_nail; // aka. nail

void model_parse();
model_t model_load(uint8_t * data, vec3_t scale);
void model_quit();

#endif
