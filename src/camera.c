#include "camera.h"
#include "globals.h"
#include "log.h"
#include "types.h"

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
    float time = (float)glfwGetTime();

    camera->position[0] = sinf(time) * 2.0f;
    camera->position[2] = cosf(time) * 2.0f;
    camera->position[1] = sinf(time * 12.0f) * 0.5f;

    camera_look_at(camera, (vec3){ 0.0f, 0.0f, 0.0f }, (vec3){ 0.0f, 1.0f, 0.0f });

    camera_update_view(camera);
    camera_update_projection(camera);
}
void camera_update_view(camera_t* camera) {
    glm_mat4_identity(camera->view);

    glm_translate(camera->view, camera->position);

    glm_rotate(camera->view, camera->rotation[0], (vec3){ 1.0f, 0.0f, 0.0f });
    glm_rotate(camera->view, camera->rotation[1], (vec3){ 0.0f, 1.0f, 0.0f });
    glm_rotate(camera->view, camera->rotation[2], (vec3){ 0.0f, 0.0f, 1.0f });

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
    glm_vec3_copy(rotation, camera->rotation);
}

void camera_translate(camera_t* camera, vec3 translation) {
    glm_vec3_add(camera->position, translation, camera->position);
}
void camera_rotate(camera_t* camera, vec3 rotation) {
    glm_vec3_add(camera->rotation, rotation, camera->rotation);
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
