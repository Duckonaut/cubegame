#pragma once

#include "types.h"
#include <cglm/types.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera.h"

#define PLAYER_HEIGHT 1.8f
#define PLAYER_RADIUS 0.3f
#define PLAYER_EYE_HEIGHT 1.62f

#define PLAYER_MAX_SPEED 5.0f
#define PLAYER_MAX_SPRINT_SPEED 10.0f
#define PLAYER_ACCELERATION 32.0f

#define PLAYER_JUMP_SPEED 6.0f
#define PLAYER_MAX_FALL_SPEED 50.0f
#define PLAYER_GRAVITY 20.0f

#define PLAYER_FRICTION 10.0f

typedef enum player_movement_mode {
    PLAYER_MOVEMENT_MODE_WALKING,
    PLAYER_MOVEMENT_MODE_FLYING,
    PLAYER_MOVEMENT_MODE_FLYING_NOCLIP,
    PLAYER_MOVEMENT_MODE_COUNT // Keep last
} player_movement_mode_t;

typedef struct player {
    player_movement_mode_t movement_mode;

    vec3 position;
    vec3 rotation;
    vec2 view_direction;
    camera_t camera;

    vec3 velocity;

    bool block_selected;
    ivec3 selected_block;
    ivec3 old_selected_block;
    vec3 selected_block_normal;
    ivec3 current_chunk;
    ivec3 old_chunk;
} player_t;

extern player_t g_player;

player_t player_new(vec3 position, vec3 rotation, camera_t camera);

void player_update(player_t* player);

void player_set_uniforms(player_t* player, shader_t* shader);
