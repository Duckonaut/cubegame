#include <cglm/cglm.h>
#include <cglm/types.h>

#include "globals.h"
#include "log.h"
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
    vec3* normal,
    float* distance
) {
    float traveled = 0.0f;
    vec3 current;
    glm_vec3_copy(r.origin, current);

    ivec3 current_block_pos;
    current_block_pos[0] = (int)floorf(current[0]);
    current_block_pos[1] = (int)floorf(current[1]);
    current_block_pos[2] = (int)floorf(current[2]);

    vec3 step;
    glm_vec3_scale(r.direction, 0.05f, step);

    vec3 end;
    glm_vec3_scale(r.direction, range, end);
    glm_vec3_add(end, r.origin, end);

    ivec3 current_chunk_pos;
    world_get_chunk_positionf(current, current_chunk_pos);

    ivec3 old_chunk_pos;
    glm_ivec3_copy(current_chunk_pos, old_chunk_pos);

    chunk_t* chunk = world_get_chunk(world, current_chunk_pos);

    ivec3 block_pos_in_chunk;

    while (traveled < range) {
        glm_vec3_add(current, step, current);
        traveled += 0.05f;

        current_block_pos[0] = (int)floorf(current[0]);
        current_block_pos[1] = (int)floorf(current[1]);
        current_block_pos[2] = (int)floorf(current[2]);

        world_get_chunk_position(current_block_pos, current_chunk_pos);

        if (current_chunk_pos[0] != old_chunk_pos[0] ||
            current_chunk_pos[1] != old_chunk_pos[1] ||
            current_chunk_pos[2] != old_chunk_pos[2]) {
            chunk = world_get_chunk(world, current_chunk_pos);
            glm_ivec3_copy(current_chunk_pos, old_chunk_pos);
        }

        if (chunk == NULL) {
            continue;
        }

        world_get_position_in_chunk(current_block_pos, block_pos_in_chunk);

        block_t* block = chunk_get_block(chunk, block_pos_in_chunk);

        if (block->id != BLOCK_AIR && (block_flags[block->id] & flags) != 0) {
            if (normal != NULL) {
                vec3 center;
                glm_vec3_add(
                    (vec3){ (float)(current_block_pos)[0],
                            (float)(current_block_pos)[1],
                            (float)(current_block_pos)[2] },
                    (vec3){ 0.5f, 0.5f, 0.5f },
                    center
                );

                vec3 diff;
                glm_vec3_sub(center, current, diff);

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

            if (distance != NULL) {
                *distance = traveled;
            }

            if (block_pos != NULL) {
                glm_ivec3_copy(current_block_pos, *block_pos);
            }

            return true;
        }
    }

    return false;
}
