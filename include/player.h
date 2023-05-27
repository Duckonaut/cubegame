#pragma once

#include "types.h"
#include <cglm/types.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera.h"

typedef struct player {
    vec3 position;
    vec3 rotation;
    vec2 view_direction;
    camera_t camera;

    bool block_selected;
    ivec3 selected_block;
    ivec3 old_selected_block;
    vec3 selected_block_normal;
} player_t;

extern player_t g_player;

player_t player_new(vec3 position, vec3 rotation, camera_t camera);

void player_update(player_t* player);

void player_set_uniforms(player_t* player, shader_t* shader);
