#pragma once

#include "shader.h"
#include "types.h"
#include <cglm/types.h>

typedef struct camera {
    vec3 position;
    vec3 rotation;
    mat4 view;
    mat4 projection;
    int screen_size[2];
} camera_t;

camera_t camera_new(vec3 position, vec3 rotation, mat4 projection);

void camera_update(camera_t* camera);
void camera_update_view(camera_t* camera);
void camera_update_projection(camera_t* camera);

void camera_set_uniforms(camera_t* camera, shader_t* shader);

void camera_set_position(camera_t* camera, vec3 position);
void camera_set_rotation(camera_t* camera, vec3 rotation);

void camera_move(camera_t* camera, vec3 offset);
void camera_rotate(camera_t* camera, vec3 rotation);

void camera_look_at(camera_t* camera, vec3 target, vec3 up);
