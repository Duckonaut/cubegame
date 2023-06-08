#include "player.h"

#include <cglm/cam.h>
#include <cglm/ivec3.h>
#include <cglm/mat4.h>
#include <cglm/quat.h>
#include <cglm/vec3.h>
#include <cglm/types.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

#include "camera.h"
#include "game.h"
#include "glm_extra.h"
#include "globals.h"
#include "log.h"
#include "physics.h"
#include "types.h"
#include "world.h"

player_t g_player;

player_t player_new(vec3 position, vec3 rotation, camera_t camera) {
    player_t player = { 0 };

    glm_vec3_copy(position, player.position);
    glm_vec3_copy(rotation, player.rotation);
    player.camera = camera;

    return player;
}

static void player_movement_flight_noclip(player_t* player) {
    vec3 facing;

    versor qy;
    glm_quatv(qy, -player->rotation[1], VEC3_UP);

    glm_quat_rotatev(qy, VEC3_FORWARD, facing);

    float speed = g_gametime.delta_time *
                  (player->movement_mode == PLAYER_MOVEMENT_MODE_FLYING_NOCLIP ? 40.0f : 10.0f);

    if (g_keyboard.keys[GLFW_KEY_W]) {
        vec3 local_facing;

        glm_vec3_scale(facing, speed, local_facing);
        glm_vec3_add(player->position, local_facing, player->position);
    }

    if (g_keyboard.keys[GLFW_KEY_S]) {
        vec3 local_facing;

        glm_vec3_scale(facing, speed, local_facing);
        glm_vec3_sub(player->position, local_facing, player->position);
    }

    if (g_keyboard.keys[GLFW_KEY_A]) {
        vec3 local_facing;

        glm_vec3_crossn(facing, VEC3_UP, local_facing);
        glm_vec3_scale(local_facing, speed, local_facing);
        glm_vec3_sub(player->position, local_facing, player->position);
    }

    if (g_keyboard.keys[GLFW_KEY_D]) {
        vec3 local_facing;

        glm_vec3_crossn(facing, VEC3_UP, local_facing);
        glm_vec3_scale(local_facing, speed, local_facing);
        glm_vec3_add(player->position, local_facing, player->position);
    }

    if (g_keyboard.keys[GLFW_KEY_SPACE]) {
        player->position[1] += speed;
    }

    if (g_keyboard.keys[GLFW_KEY_LEFT_SHIFT]) {
        player->position[1] -= speed;
    }
}

static void player_movement_normal(player_t* player) {
    vec3 facing;

    versor qy;
    glm_quatv(qy, -player->rotation[1], VEC3_UP);

    glm_quat_rotatev(qy, VEC3_FORWARD, facing);

    vec3 facing_flat = { facing[0], 0.0f, facing[2] };
    glm_vec3_normalize(facing_flat);

    bool on_ground = false;
    ivec3 block_pos;

    if (player->velocity[1] <= 0.0f) {
        vec3 start = { player->position[0], player->position[1], player->position[2] };
        f32 cast_distance = 0.01f;

        ray_t ray;
        glm_vec3_copy(start, ray.origin);
        ray.direction[0] = 0.0f;
        ray.direction[1] = -1.0f;
        ray.direction[2] = 0.0f;

        if (ray_intersect_block(
                ray,
                g_game.world,
                cast_distance,
                BLOCK_FLAG_SOLID,
                &block_pos,
                NULL,
                NULL
            )) {
            on_ground = true;
        }
    }

    bool any_movement = g_keyboard.keys[GLFW_KEY_W] || g_keyboard.keys[GLFW_KEY_S] ||
                        g_keyboard.keys[GLFW_KEY_A] || g_keyboard.keys[GLFW_KEY_D];

    if (g_keyboard.keys[GLFW_KEY_W]) {
        vec3 local_facing;

        glm_vec3_scale(facing_flat, g_gametime.delta_time * PLAYER_ACCELERATION, local_facing);
        glm_vec3_add(player->velocity, local_facing, player->velocity);
    }

    if (g_keyboard.keys[GLFW_KEY_S]) {
        vec3 local_facing;

        glm_vec3_scale(facing_flat, g_gametime.delta_time * PLAYER_ACCELERATION, local_facing);
        glm_vec3_sub(player->velocity, local_facing, player->velocity);
    }

    if (g_keyboard.keys[GLFW_KEY_A]) {
        vec3 local_facing;

        glm_vec3_crossn(facing_flat, VEC3_UP, local_facing);
        glm_vec3_scale(local_facing, g_gametime.delta_time * PLAYER_ACCELERATION, local_facing);
        glm_vec3_sub(player->velocity, local_facing, player->velocity);
    }

    if (g_keyboard.keys[GLFW_KEY_D]) {
        vec3 local_facing;

        glm_vec3_crossn(facing, VEC3_UP, local_facing);
        glm_vec3_scale(local_facing, g_gametime.delta_time * PLAYER_ACCELERATION, local_facing);
        glm_vec3_add(player->velocity, local_facing, player->velocity);
    }

    if (on_ground) {
        player->velocity[1] = 0.0f;

        player->position[1] =
            (f32)block_pos[1] + 1.0f; // block-space (0, 0, 0) is at the bottom of the block

        if (g_keyboard.keys[GLFW_KEY_SPACE]) {
            player->velocity[1] = PLAYER_JUMP_SPEED;
        }

        if (!any_movement) {
            vec3 horizontal_velocity = { player->velocity[0], 0.0f, player->velocity[2] };
            glm_vec3_scale(
                horizontal_velocity,
                1.0f - g_gametime.delta_time * PLAYER_FRICTION,
                horizontal_velocity
            );
            player->velocity[0] = horizontal_velocity[0];
            player->velocity[2] = horizontal_velocity[2];
        }
    } else {
        player->velocity[1] -= g_gametime.delta_time * PLAYER_GRAVITY;
    }

    f32 horizontal_speed =
        glm_vec3_norm((vec3){ player->velocity[0], 0.0f, player->velocity[2] });

    if (horizontal_speed > PLAYER_MAX_SPEED) {
        vec3 horizontal_velocity = { player->velocity[0], 0.0f, player->velocity[2] };
        glm_vec3_scale_as(horizontal_velocity, PLAYER_MAX_SPEED, horizontal_velocity);
        player->velocity[0] = horizontal_velocity[0];
        player->velocity[2] = horizontal_velocity[2];
    }

    f32 vertical_speed = player->velocity[1];

    if (vertical_speed > PLAYER_MAX_FALL_SPEED) {
        player->velocity[1] = PLAYER_MAX_SPEED;
    }

    vec3 displacement;
    glm_vec3_scale(player->velocity, g_gametime.delta_time, displacement);

    // check collision
    vec3 origin = { player->position[0], player->position[1] + 0.5f, player->position[2] };
    f32 cast_distance = glm_vec3_norm(displacement) + 0.02f;

    ray_t ray;
    glm_vec3_copy(origin, ray.origin);

    vec3 hit_normal;

    static const vec3 DIRECTIONS[] = {
        { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },  // left, right
        { 0.0f, 0.0f, 1.0f },  { 0.0f, 0.0f, -1.0f }, // forward, back
        { 0.0f, 1.0f, 0.0f },                         // up, ignore down
    };

    for (u32 i = 0; i < 5; i++) {
        if (glm_vec3_norm(displacement) < 0.0001f) {
            break;
        }

        if (i == 2) {
            glm_vec3_add(origin, (vec3){ 0.0f, 1.3f, 0.0f }, ray.origin);
        }

        glm_vec3_copy((f32*)DIRECTIONS[i], ray.direction);

        ivec3 block_pos;
        float hit_distance;
        if (ray_intersect_block(
                ray,
                g_game.world,
                cast_distance,
                BLOCK_FLAG_SOLID,
                &block_pos,
                &hit_normal,
                &hit_distance
            )) {
            f32 dot = glm_vec3_dot(displacement, hit_normal);
            if (dot < 0.0f) {
                // remove the displacement that is in the direction of the hit normal
                vec3 displacement_along_normal;
                glm_vec3_scale(hit_normal, dot, displacement_along_normal);
                glm_vec3_sub(displacement, displacement_along_normal, displacement);

                // also remove the component of velocity that is in the direction of the hit
                // normal
                glm_vec3_scale(
                    hit_normal,
                    glm_vec3_dot(player->velocity, hit_normal),
                    hit_normal
                );
                glm_vec3_sub(player->velocity, hit_normal, player->velocity);
            }
        }
    }

    for (u32 i = 0; i < 5; i++) {
        if (glm_vec3_norm(displacement) < 0.0001f) {
            break;
        }

        glm_vec3_copy((f32*)DIRECTIONS[i], ray.direction);

        ivec3 block_pos;
        float hit_distance;
        if (ray_intersect_block(
                ray,
                g_game.world,
                cast_distance,
                BLOCK_FLAG_SOLID,
                &block_pos,
                &hit_normal,
                &hit_distance
            )) {
            f32 dot = glm_vec3_dot(displacement, hit_normal);
            if (dot < 0.0f) {
                // remove the displacement that is in the direction of the hit normal
                vec3 displacement_along_normal;
                glm_vec3_scale(hit_normal, dot, displacement_along_normal);
                glm_vec3_sub(displacement, displacement_along_normal, displacement);

                // also remove the component of velocity that is in the direction of the hit
                // normal
                glm_vec3_scale(
                    hit_normal,
                    glm_vec3_dot(player->velocity, hit_normal),
                    hit_normal
                );
                glm_vec3_sub(player->velocity, hit_normal, player->velocity);
            }
        }
    }

    glm_vec3_add(player->position, displacement, player->position);
}

static void player_movement(player_t* player) {
    switch (player->movement_mode) {
        case PLAYER_MOVEMENT_MODE_FLYING:
        case PLAYER_MOVEMENT_MODE_FLYING_NOCLIP:
            player_movement_flight_noclip(player);
            break;
        case PLAYER_MOVEMENT_MODE_WALKING:
            player_movement_normal(player);
            break;
        default:
            break;
    }
}

static void player_look(player_t* player) {
    if (!g_mouse.captured) {
        return;
    }
    player->view_direction[0] += g_mouse.delta[1] * 0.2f * g_gametime.delta_time;
    player->view_direction[1] += g_mouse.delta[0] * 0.2f * g_gametime.delta_time;

    if (player->view_direction[0] > GLM_PI_2f - 0.05f) {
        player->view_direction[0] = GLM_PI_2f - 0.05f;
    } else if (player->view_direction[0] < -GLM_PI_2f + 0.05f) {
        player->view_direction[0] = -GLM_PI_2f + 0.05f;
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

    vec3 camera_position;
    glm_vec3_add(player->position, (vec3){ 0.0f, PLAYER_EYE_HEIGHT, 0.0f }, camera_position);
    camera_set_position(&player->camera, camera_position);

    camera_update(&player->camera);

    world_get_chunk_positionf(player->position, player->current_chunk);

    if (g_debug_tools.no_chunk_load) {
        return;
    }

    if (player->current_chunk[0] != player->old_chunk[0] ||
        player->current_chunk[1] != player->old_chunk[1] ||
        player->current_chunk[2] != player->old_chunk[2]) {
        glm_ivec3_copy(player->current_chunk, player->old_chunk);

        ivec3 chunk_pos;
        for (int x = -2; x <= 2; x++) {
            for (int z = -2; z <= 2; z++) {
                int min_y = player->current_chunk[1] - 2;
                if (min_y > 0) {
                    min_y = 0;
                }

                int max_y = player->current_chunk[1] + 2;

                for (int y = min_y; y <= max_y; y++) {
                    glm_ivec3_copy(player->current_chunk, chunk_pos);
                    chunk_pos[0] += x;
                    chunk_pos[1] = y;
                    chunk_pos[2] += z;

                    chunk_t* c = world_get_or_load_chunk(g_game.world, chunk_pos);

                    if (c) {
                        LOG_DEBUG(
                            "Loaded chunk %d %d %d\n",
                            chunk_pos[0],
                            chunk_pos[1],
                            chunk_pos[2]
                        );
                    }
                }
            }
        }
    }
}

void player_set_uniforms(player_t* player, shader_t* shader) {
    shader_set_mat4(shader, "u_view", player->camera.view);
    shader_set_mat4(shader, "u_projection", player->camera.projection);
    shader_set_vec3(shader, "u_world_eye", player->camera.position);
}
