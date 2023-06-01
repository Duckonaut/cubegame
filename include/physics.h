#pragma once

#include <cglm/cglm.h>
#include <cglm/types.h>

#include "camera.h"
#include "math.h"
#include "glm_extra.h"
#include "world.h"

typedef struct ray {
    vec3 origin;
    vec3 direction;
} ray_t;

void ray_from_camera(ray_t* ray, camera_t* camera);

bool ray_intersect_block(
    ray_t r,
    world_t* world,
    float range,
    block_flags_t flags,
    ivec3* block_pos,
    vec3* normal,
    float* distance
);
