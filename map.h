
#ifndef MAP_H
#define MAP_H

#include <stdint.h>

#include "math.h"
#include "vector.h"

#define map_size 128

typedef struct {
    uint32_t t;
    int32_t b;
} block_t;

typedef struct {
    size_t vert_len;
    size_t frame_len;
    char (*frame_names)[][100];
    vector * frames;
    uint32_t tex_id;
    size_t name_len;
    char entity_name[100];
    vec3_t size; // min/max bounds of vertices
} ref_entt_t;

// todo, move entity table and init to another file?
typedef enum {
    ENTITY_ID_PLAYER,
    ENTITY_ID_ENEMY_GRUNT,
    ENTITY_ID_ENEMY_MUTANT,
    ENTITY_ID_ENEMY_ENFORCER,
    ENTITY_ID_ENEMY_OGRE,
    ENTITY_ID_ENEMY_ZOMBIE,
    ENTITY_ID_ENEMY_HOUND,
    ENTITY_ID_PICKUP_SHOTGUN,
    ENTITY_ID_PICKUP_NAILGUN,
    ENTITY_ID_PICKUP_GRENADELAUNCHER,
    ENTITY_ID_PICKUP_HEALTH,
    ENTITY_ID_PICKUP_NAILS,
    ENTITY_ID_PICKUP_GRENADES,
    ENTITY_ID_PICKUP_KEY,
    ENTITY_ID_PROJECTILE_GRENADE,
    ENTITY_ID_PROJECTILE_NAIL,
    ENTITY_ID_PROJECTILE_PLASMA,
    ENTITY_ID_PROJECTILE_SHELL,
    ENTITY_ID_BARREL,
    ENTITY_ID_LIGHT,
    ENTITY_ID_TRIGGER_LEVEL,
    ENTITY_ID_DOOR,
    ENTITY_ID_TORCH,
    __ENTITY_ID_END,
} entity_id_t;

// move the following

typedef struct {
    char k[100];
    char v[100];
} entity_extra_params_t;

typedef struct {
    vec3_t position;
} entity_player_params_t;

typedef struct {
    vec3_t position;
    uint8_t rgba[4];
} entity_light_params_t;

typedef struct {
    vec3_t position;
    ref_entt_t * ref_entt;
    vector * extras;
} entity_generic_params_t;

// to here, back to their header files or some shit
// along with I guess some parsing info

typedef struct {
    entity_id_t id;
    union {
        entity_player_params_t entity_player_params;
        entity_light_params_t entity_light_params;
        entity_generic_params_t entity_generic_params;
    };
} entity_params_t;

typedef struct {
    vector * blocks;
    uint8_t * e;
    // map_entities and ref_entities is for new map format only
    vector * map_entities; // entity_params_t
    vector * ref_entities; // ref_entt_t
    vector * ref_ent_index; // size_t
    uint32_t e_size;
    uint8_t cm[((map_size * map_size * map_size) >> 3)];
} map_t;

extern vector * map_data;

void map_init();
void map_parse();
void map_load (map_t * m);
uint8_t map_block_at(int32_t x, int32_t y, int32_t z);
int map_block_at_box(vec3_t box_start, vec3_t box_end);
void map_draw();
bool map_trace(vec3_t a, vec3_t b);
void map_quit();
ref_entt_t * map_ref_entt_from_name(char * name);
ref_entt_t * map_ref_entt_from_eid(entity_id_t eid);
entity_params_t map_entt_params_from_eid(entity_id_t eid);

#endif
