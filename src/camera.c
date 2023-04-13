#include "camera.h"
#include "glm_extra.h"
#include "globals.h"
#include "log.h"
#include "mesh.h"
#include "types.h"
#include "world.h"

#include <cglm/cam.h>
#include <cglm/cglm.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/quat.h>
#include <cglm/types.h>
#include <math.h>

camera_t camera_new(vec3 position, vec3 rotation, mat4 projection) {
    camera_t camera = { 0 };

    glm_vec3_copy(position, camera.position);
    glm_vec3_copy(rotation, camera.rotation);
    glm_mat4_copy(projection, camera.projection);

    camera.screen_size[0] = g_window_size[0];
    camera.screen_size[1] = g_window_size[1];

    return camera;
}

void camera_update(camera_t* camera) {
    camera_update_view(camera);
    camera_update_projection(camera);
}
void camera_update_view(camera_t* camera) {
    glm_mat4_identity(camera->view);

    glm_translate(camera->view, camera->position);

    glm_quat_rotate(camera->view, camera->rotation, camera->view);

    glm_mat4_inv(camera->view, camera->view);
}
void camera_update_projection(camera_t* camera) {
    if (g_window_size[0] != camera->screen_size[0] ||
        g_window_size[1] != camera->screen_size[1]) {
        camera->screen_size[0] = g_window_size[0];
        camera->screen_size[1] = g_window_size[1];

        float aspect = (float)g_window_size[0] / (float)g_window_size[1];
        glm_perspective_resize(aspect, camera->projection);
    }
}

void camera_set_position(camera_t* camera, vec3 position) {
    glm_vec3_copy(position, camera->position);
}
void camera_set_rotation(camera_t* camera, vec3 rotation) {
    glm_quat_identity(camera->rotation);
    camera_rotate(camera, rotation);
}

void camera_translate(camera_t* camera, vec3 translation) {
    glm_vec3_add(camera->position, translation, camera->position);
}
void camera_rotate(camera_t* camera, vec3 rotation) {
    versor qx, qy, qz;
    glm_quatv(qx, rotation[0], (vec3){ 1.0f, 0.0f, 0.0f });
    glm_quatv(qy, rotation[1], (vec3){ 0.0f, 1.0f, 0.0f });
    glm_quatv(qz, rotation[2], (vec3){ 0.0f, 0.0f, 1.0f });
    glm_quat_mul(qx, qy, camera->rotation);
    glm_quat_mul(camera->rotation, qz, camera->rotation);
}

void camera_look_at(camera_t* camera, vec3 target, vec3 up) {
    glm_lookat(camera->position, target, up, camera->view);

    // Extract euler angles from view matrix
    camera->rotation[0] = atan2f(camera->view[1][2], camera->view[2][2]);
    camera->rotation[1] = atan2f(
        -camera->view[0][2],
        sqrtf(camera->view[1][2] * camera->view[1][2] + camera->view[2][2] * camera->view[2][2])
    );
    camera->rotation[2] = atan2f(camera->view[0][1], camera->view[0][0]);

    // Invert the rotation
    camera->rotation[0] = -camera->rotation[0];
    camera->rotation[1] = -camera->rotation[1];
    camera->rotation[2] = -camera->rotation[2];
}

void camera_screen_to_world(
    camera_t* camera,
    vec2 screen_pos,
    vec3* world_position,
    vec3* world_direction
) {
    // Convert screen position to normalized device coordinates
    // using the camera's projection and view matrices

    // Calculate view projection matrix
    mat4 view_projection = { 0 };
    glm_mat4_mul(camera->projection, camera->view, view_projection);

    // Unproject the screen position to world position
    vec4 world_pos = { 0 };

    glm_unproject(
        (vec3){ screen_pos[0], screen_pos[1], 0 },
        view_projection,
        (vec4){ 0, 0, (float)g_window_size[0], (float)g_window_size[1] },
        world_pos
    );

    glm_vec3_copy(world_pos, *world_position);

    glm_quat_rotatev(camera->rotation, VEC3_FORWARD, *world_direction);
}

bool world_get_block_at(const ivec3 position, ivec3* block_position) {
    if (position[0] < 0 || position[0] >= 16 || position[1] < 0 || position[1] >= 16 ||
        position[2] < 0 || position[2] >= 16) {
        return false;
    }

    (*block_position)[0] = position[0];
    (*block_position)[1] = position[1];
    (*block_position)[2] = position[2];

    return true;
}

bool world_get_block(const vec3 position, ivec3* block_position) {
    (*block_position)[0] = (int)floorf(position[0] + 0.5f);
    (*block_position)[1] = (int)floorf(position[1] + 0.5f);
    (*block_position)[2] = (int)floorf(position[2] + 0.5f);

    return world_get_block_at(*block_position, block_position);
}

bool camera_pointed_block(
    camera_t* camera,
    float range,
    const mesh_instance_t* blocks,
    ivec3* block_position
) {
    vec3 ray_origin = { 0 };
    vec3 ray_direction = { 0 };

    camera_screen_to_world(
        camera,
        (vec2){ (float)g_window_size[0] / 2, (float)g_window_size[1] / 2 },
        &ray_origin,
        &ray_direction
    );

    vec3 ray_end = { 0 };
    glm_vec3_scale(ray_direction, range, ray_end);
    glm_vec3_add(ray_origin, ray_end, ray_end);

    vec3 ray_step = { 0 };
    glm_vec3_scale(ray_direction, 0.1f, ray_step);

    vec3 ray_current = { 0 };
    glm_vec3_copy(ray_origin, ray_current);

    while (glm_vec3_distance(ray_current, ray_end) > 0.1f) {
        glm_vec3_add(ray_current, ray_step, ray_current);

        if (world_get_block(ray_current, block_position)) {
            usize block_index = (usize)(*block_position)[0] +
                                (usize)(*block_position)[1] * CHUNK_SIZE +
                                (usize)(*block_position)[2] * CHUNK_SIZE * CHUNK_SIZE;

            if (blocks[block_index].active) {
                return true;
            }
        }
    }

    return false;
}
