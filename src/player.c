#include "player.h"

#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <cglm/quat.h>
#include <cglm/vec3.h>
#include <cglm/types.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "glm_extra.h"
#include "globals.h"

player_t g_player;

player_t player_new(vec3 position, vec3 rotation, camera_t camera) {
    player_t player = { 0 };

    glm_vec3_copy(position, player.position);
    glm_vec3_copy(rotation, player.rotation);
    player.camera = camera;

    return player;
}

static void player_movement(player_t* player) {
    vec3 facing;

    versor qy;
    glm_quatv(qy, -player->rotation[1], VEC3_UP);

    glm_quat_rotatev(qy, VEC3_FORWARD, facing);

    if (g_keyboard.keys[GLFW_KEY_W]) {
        vec3 local_facing;

        glm_vec3_scale(facing, g_gametime.delta_time * 10.0f, local_facing);
        glm_vec3_add(player->position, local_facing, player->position);
    }

    if (g_keyboard.keys[GLFW_KEY_S]) {
        vec3 local_facing;

        glm_vec3_scale(facing, g_gametime.delta_time * 10.0f, local_facing);
        glm_vec3_sub(player->position, local_facing, player->position);
    }

    if (g_keyboard.keys[GLFW_KEY_A]) {
        vec3 local_facing;

        glm_vec3_crossn(facing, VEC3_UP, local_facing);
        glm_vec3_scale(local_facing, g_gametime.delta_time * 10.0f, local_facing);
        glm_vec3_sub(player->position, local_facing, player->position);
    }

    if (g_keyboard.keys[GLFW_KEY_D]) {
        vec3 local_facing;

        glm_vec3_crossn(facing, VEC3_UP, local_facing);
        glm_vec3_scale(local_facing, g_gametime.delta_time * 10.0f, local_facing);
        glm_vec3_add(player->position, local_facing, player->position);
    }

    if (g_keyboard.keys[GLFW_KEY_SPACE]) {
        player->position[1] += g_gametime.delta_time * 10.0f;
    }

    if (g_keyboard.keys[GLFW_KEY_LEFT_SHIFT]) {
        player->position[1] -= g_gametime.delta_time * 10.0f;
    }
}

static void player_look(player_t* player) {
    if (!g_mouse.captured) {
        return;
    }
    player->view_direction[0] += g_mouse.delta[1] * 0.2f * g_gametime.delta_time;
    player->view_direction[1] += g_mouse.delta[0] * 0.2f * g_gametime.delta_time;

    if (player->view_direction[0] > GLM_PI_2f - 0.1f) {
        player->view_direction[0] = GLM_PI_2f - 0.1f;
    } else if (player->view_direction[0] < -GLM_PI_2f + 0.1f) {
        player->view_direction[0] = -GLM_PI_2f + 0.1f;
    }

    player->rotation[0] = player->view_direction[0];
    player->rotation[1] = player->view_direction[1];

    glm_quat_identity(player->camera.rotation);
    versor qy, qx;
    glm_quatv(qy, -player->rotation[1], VEC3_UP);
    glm_quatv(qx, -player->rotation[0], VEC3_RIGHT);

    glm_quat_mul(qy, qx, player->camera.rotation);
}

void player_update(player_t* player) {
    player_look(player);
    player_movement(player);

    camera_set_position(&player->camera, player->position);

    camera_update(&player->camera);
}

void player_set_uniforms(player_t* player, shader_t* shader) {
    shader_set_mat4(shader, "u_view", player->camera.view);
    shader_set_mat4(shader, "u_projection", player->camera.projection);
    shader_set_vec3(shader, "u_world_eye", player->camera.position);
}
