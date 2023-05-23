#include <cglm/cglm.h>
#include <cglm/types.h>

#include "globals.h"
#include "physics.h"
#include "world.h"

void ray_from_camera(ray_t* ray, camera_t* camera) {
    camera_screen_to_world(
        camera,
        (vec2){ (float)g_window_size[0] / 2, (float)g_window_size[1] / 2 },
        &ray->origin,
        &ray->direction
    );
}

bool ray_intersect_block(
    ray_t r,
    world_t* world,
    float range,
    block_flags_t flags,
    ivec3* block_pos,
    vec3* normal
) {
    vec3 current;
    glm_vec3_copy(r.origin, current);

    vec3 step;
    glm_vec3_scale(r.direction, 0.1f, step);

    vec3 end;
    glm_vec3_scale(r.direction, range, end);
    glm_vec3_add(end, r.origin, end);

    while (glm_vec3_distance(current, end) > 0.1f) {
        glm_vec3_add(current, step, current);

        (*block_pos)[0] = (int)current[0];
        (*block_pos)[1] = (int)current[1];
        (*block_pos)[2] = (int)current[2];

        block_t* block = world_get_block_at(world, *block_pos);

        if (block == NULL) {
            continue;
        }

        if (block->id != BLOCK_AIR && (block_flags[block->id] * flags) != 0) {
            if (normal != NULL) {
                vec3 center;
                glm_vec3_add(current, (vec3){ 0.5f, 0.5f, 0.5f }, center);

                vec3 diff;
                glm_vec3_sub(center, r.origin, diff);

                vec3 abs_diff;
                glm_vec3_abs(diff, abs_diff);

                if (abs_diff[0] > abs_diff[1] && abs_diff[0] > abs_diff[2]) {
                    (*normal)[0] = diff[0] > 0 ? -1 : 1;
                    (*normal)[1] = 0;
                    (*normal)[2] = 0;
                } else if (abs_diff[1] > abs_diff[0] && abs_diff[1] > abs_diff[2]) {
                    (*normal)[0] = 0;
                    (*normal)[1] = diff[1] > 0 ? -1 : 1;
                    (*normal)[2] = 0;
                } else {
                    (*normal)[0] = 0;
                    (*normal)[1] = 0;
                    (*normal)[2] = diff[2] > 0 ? -1 : 1;
                }
            }


            return true;
        }
    }

    return false;
}
