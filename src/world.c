#include "world.h"

#include "types.h"
#include "assets.h"
#include "glm_extra.h"
#include "log.h"
#include "mesh.h"
#include "player.h"
#include "shader.h"
#include "utils.h"

#include <assert.h>
#include <cglm/affine-pre.h>
#include <cglm/ivec3.h>
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <cglm/vec3.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

block_flags_t block_flags[BLOCK_ID_MAX] = {
    // BLOCK_ID_AIR
    (block_flags_t)BLOCK_FLAG_TRANSPARENT,
    // BLOCK_ID_STONE
    (block_flags_t)(BLOCK_FLAG_SOLID | BLOCK_FLAG_MESHED),
    // BLOCK_ID_DIRT
    (block_flags_t)(BLOCK_FLAG_SOLID | BLOCK_FLAG_MESHED),
    // BLOCK_ID_GRASS
    (block_flags_t)(BLOCK_FLAG_SOLID | BLOCK_FLAG_MESHED | BLOCK_FLAG_TEXTURE_TOP |
                    BLOCK_FLAG_TEXTURE_BOTTOM),
    // BLOCK_ID_BRICK
    (block_flags_t)(BLOCK_FLAG_SOLID | BLOCK_FLAG_MESHED),
    // BLOCK_ID_GLASS
    (block_flags_t)(BLOCK_FLAG_SOLID | BLOCK_FLAG_MESHED | BLOCK_FLAG_TRANSPARENT),
};

void chunk_init(chunk_t* chunk, world_t* world, ivec3 position) {
    glm_ivec3_copy(position, chunk->position);
    for (int i = 0; i < CHUNK_SIZE; i++) {
        chunk->blocks[i] = (block_t){ .id = BLOCK_AIR };
    }
    chunk->mesh.vertices = NULL;
    chunk->mesh.indices = NULL;

    chunk_generate(chunk);
    chunk_mesh(chunk, world);
}
void chunk_forget(chunk_t* chunk) {
    chunk_forget_mesh(chunk);
    free(chunk->mesh.vertices);
    free(chunk->mesh.indices);
}

void chunk_generate(chunk_t* chunk) {
    for (i32 x = 0; x < CHUNK_SIZE; x++) {
        for (i32 z = 0; z < CHUNK_SIZE; z++) {
            f32 realx = (f32)(x + chunk->position[0] * CHUNK_SIZE);
            f32 realz = (f32)(z + chunk->position[2] * CHUNK_SIZE);

            i32 height = 10 + (int)(perlin2d(realx * 0.05f, realz * 0.05f) * 10.0f) +
                         (int)(perlin2d(realx * 0.01f, realz * 0.01f) * 30.0f);

            i32 rock_height = height - 5;

            for (i32 y = 0; y < CHUNK_SIZE; y++) {
                i32 index = CHUNK_POS_TO_INDEX(x, y, z);
                i32 world_y = y + chunk->position[1] * CHUNK_SIZE;

                float cave_noise = perlin3d(realx * 0.1f, (f32)world_y * 0.1f, realz * 0.1f);

                float cave_factor = 0.4f;

                if (world_y > height - 10) {
                    cave_factor = 0.4f + ((float)world_y - ((float)height - 10)) * 0.06f;
                }

                if (cave_noise > cave_factor) {
                    chunk->blocks[index].id = BLOCK_AIR;
                } else if (world_y == height) {
                    chunk->blocks[index].id = 3;
                } else if (world_y < rock_height) {
                    chunk->blocks[index].id = 1;
                } else if (world_y < height) {
                    chunk->blocks[index].id = 2;
                } else {
                    chunk->blocks[index].id = BLOCK_AIR;
                }
            }
        }
    }
}

void chunk_set_block(chunk_t* chunk, world_t* world, ivec3 position, block_id_t id) {
    i32 index = CHUNK_POS_TO_INDEX(position[0], position[1], position[2]);
    chunk->blocks[index].id = id;
    world_remesh_queue_add(world, (u32)(chunk - world->chunks));
}

block_t* chunk_get_block(chunk_t* chunk, ivec3 position) {
    if (position[0] < 0 || position[0] >= CHUNK_SIZE || position[1] < 0 ||
        position[1] >= CHUNK_SIZE || position[2] < 0 || position[2] >= CHUNK_SIZE) {
        LOG_ERROR("Invalid block position: %d, %d, %d", position[0], position[1], position[2]);
        assert(false);
    }

    i32 index = CHUNK_POS_TO_INDEX(position[0], position[1], position[2]);
    return &chunk->blocks[index];
}

void chunk_mesh(chunk_t* chunk, world_t* world) {
    if (!chunk->mesh.vertices) {
        chunk->mesh.vertices =
            malloc(sizeof(vertex_t) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 4);
    }
    if (!chunk->mesh.indices) {
        chunk->mesh.indices =
            malloc(sizeof(u32) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 6);
    }

    chunk->mesh.vertex_count = 0;
    chunk->mesh.index_count = 0;

    chunk_t* neighbors[6];
    neighbors[0] = world_get_chunk(
        world,
        (ivec3){ chunk->position[0] - 1, chunk->position[1], chunk->position[2] }
    );
    neighbors[1] = world_get_chunk(
        world,
        (ivec3){ chunk->position[0] + 1, chunk->position[1], chunk->position[2] }
    );
    neighbors[2] = world_get_chunk(
        world,
        (ivec3){ chunk->position[0], chunk->position[1] - 1, chunk->position[2] }
    );
    neighbors[3] = world_get_chunk(
        world,
        (ivec3){ chunk->position[0], chunk->position[1] + 1, chunk->position[2] }
    );
    neighbors[4] = world_get_chunk(
        world,
        (ivec3){ chunk->position[0], chunk->position[1], chunk->position[2] - 1 }
    );
    neighbors[5] = world_get_chunk(
        world,
        (ivec3){ chunk->position[0], chunk->position[1], chunk->position[2] + 1 }
    );

    for (i32 x = 0; x < CHUNK_SIZE; x++) {
        for (i32 y = 0; y < CHUNK_SIZE; y++) {
            for (i32 z = 0; z < CHUNK_SIZE; z++) {
                chunk_remesh_block(chunk, world, neighbors, (ivec3){ x, y, z });
            }
        }
    }

    chunk->mesh.draw_mode = GL_TRIANGLES;

    mesh_init(&chunk->mesh);
}

void chunk_forget_mesh(chunk_t* chunk) {
    if (chunk->mesh.vao) {
        mesh_free(&chunk->mesh);
    }
    chunk->mesh.vertex_count = 0;
    chunk->mesh.index_count = 0;
}

void chunk_remesh(chunk_t* chunk, world_t* world) {
    chunk_forget_mesh(chunk);
    chunk_mesh(chunk, world);
}

void chunk_remesh_block(
    chunk_t* chunk,
    world_t* world,
    chunk_t* neighbor_chunks[6],
    ivec3 pos
) {
    block_t* block = chunk_get_block(chunk, pos);
    if (block->id == BLOCK_AIR) {
        return;
    }

    block_flags_t flags = block_flags[block->id];
    if (!(flags & BLOCK_FLAG_MESHED)) {
        return;
    }

    ivec3 block_pos = { 0 };
    glm_ivec3_copy(pos, block_pos);

    static ivec3 DIRECTION_OFFSETS[6] = {
        { -1, 0, 0 }, { 1, 0, 0 }, { 0, -1, 0 }, { 0, 1, 0 }, { 0, 0, -1 }, { 0, 0, 1 },
    };

    block->mesh_index_offset = chunk->mesh.index_count;
    block->mesh_vertex_offset = chunk->mesh.vertex_count;

    for (i32 i = 0; i < 6; i++) {
        ivec3 neighbor_pos = { 0 };
        glm_ivec3_add(block_pos, DIRECTION_OFFSETS[i], neighbor_pos);

        if (neighbor_pos[0] < 0 || neighbor_pos[0] >= CHUNK_SIZE || neighbor_pos[1] < 0 ||
            neighbor_pos[1] >= CHUNK_SIZE || neighbor_pos[2] < 0 ||
            neighbor_pos[2] >= CHUNK_SIZE) {
            if (world != NULL) {
                chunk_t* neighbor = neighbor_chunks[i];

                if (neighbor == NULL) {
                    block_mesh_face(&chunk->mesh, block_pos, (block_face_t)i, block);
                    continue;
                }

                ivec3 neighbor_block_world_pos = { 0 };
                glm_ivec3_copy(chunk->position, neighbor_block_world_pos);
                glm_ivec3_scale(neighbor_block_world_pos, CHUNK_SIZE, neighbor_block_world_pos);
                glm_ivec3_add(neighbor_block_world_pos, neighbor_pos, neighbor_block_world_pos);

                ivec3 neighbor_chunk_pos;
                world_get_position_in_chunk(neighbor_block_world_pos, neighbor_chunk_pos);

                block_t* neighbor_block = chunk_get_block(neighbor, neighbor_chunk_pos);

                if (neighbor_block->id == BLOCK_AIR ||
                    (block_flags[neighbor_block->id] & BLOCK_FLAG_TRANSPARENT)) {
                    block_mesh_face(&chunk->mesh, block_pos, (block_face_t)i, block);
                }
            } else {
                block_mesh_face(&chunk->mesh, block_pos, (block_face_t)i, block);
            }
            continue;
        }

        block_t* neighbor = chunk_get_block(chunk, neighbor_pos);
        if (neighbor->id == BLOCK_AIR || (block_flags[neighbor->id] & BLOCK_FLAG_TRANSPARENT)) {
            block_mesh_face(&chunk->mesh, block_pos, (block_face_t)i, block);
        }
    }

    block->mesh_index_count = chunk->mesh.index_count - block->mesh_index_offset;
    block->mesh_vertex_count = chunk->mesh.vertex_count - block->mesh_vertex_offset;
}

void block_mesh_face(mesh_t* mesh, ivec3 position, block_face_t face, block_t* block) {
    block_flags_t flags = block_flags[block->id];

    vertex_t vertices[4] = { 0 };

    switch (face) {
        case BLOCK_FACE_LEFT:
            vertices[0].position[0] = (float)position[0];
            vertices[0].position[1] = (float)position[1];
            vertices[0].position[2] = (float)position[2] + 1.0f;

            vertices[1].position[0] = (float)position[0];
            vertices[1].position[1] = (float)position[1] + 1.0f;
            vertices[1].position[2] = (float)position[2] + 1.0f;

            vertices[2].position[0] = (float)position[0];
            vertices[2].position[1] = (float)position[1] + 1.0f;
            vertices[2].position[2] = (float)position[2];

            vertices[3].position[0] = (float)position[0];
            vertices[3].position[1] = (float)position[1];
            vertices[3].position[2] = (float)position[2];

            for (i32 i = 0; i < 4; i++) {
                vertices[i].normal[0] = -1.0f;
                vertices[i].normal[1] = 0.0f;
                vertices[i].normal[2] = 0.0f;
            }
            break;
        case BLOCK_FACE_RIGHT:
            vertices[0].position[0] = (float)position[0] + 1.0f;
            vertices[0].position[1] = (float)position[1];
            vertices[0].position[2] = (float)position[2];

            vertices[1].position[0] = (float)position[0] + 1.0f;
            vertices[1].position[1] = (float)position[1] + 1.0f;
            vertices[1].position[2] = (float)position[2];

            vertices[2].position[0] = (float)position[0] + 1.0f;
            vertices[2].position[1] = (float)position[1] + 1.0f;
            vertices[2].position[2] = (float)position[2] + 1.0f;

            vertices[3].position[0] = (float)position[0] + 1.0f;
            vertices[3].position[1] = (float)position[1];
            vertices[3].position[2] = (float)position[2] + 1.0f;

            for (i32 i = 0; i < 4; i++) {
                vertices[i].normal[0] = 1.0f;
                vertices[i].normal[1] = 0.0f;
                vertices[i].normal[2] = 0.0f;
            }
            break;
        case BLOCK_FACE_BOTTOM:
            vertices[0].position[0] = (float)position[0];
            vertices[0].position[1] = (float)position[1];
            vertices[0].position[2] = (float)position[2];

            vertices[1].position[0] = (float)position[0] + 1.0f;
            vertices[1].position[1] = (float)position[1];
            vertices[1].position[2] = (float)position[2];

            vertices[2].position[0] = (float)position[0] + 1.0f;
            vertices[2].position[1] = (float)position[1];
            vertices[2].position[2] = (float)position[2] + 1.0f;

            vertices[3].position[0] = (float)position[0];
            vertices[3].position[1] = (float)position[1];
            vertices[3].position[2] = (float)position[2] + 1.0f;

            for (i32 i = 0; i < 4; i++) {
                vertices[i].normal[0] = 0.0f;
                vertices[i].normal[1] = -1.0f;
                vertices[i].normal[2] = 0.0f;
            }
            break;
        case BLOCK_FACE_TOP:
            vertices[0].position[0] = (float)position[0];
            vertices[0].position[1] = (float)position[1] + 1.0f;
            vertices[0].position[2] = (float)position[2];

            vertices[1].position[0] = (float)position[0];
            vertices[1].position[1] = (float)position[1] + 1.0f;
            vertices[1].position[2] = (float)position[2] + 1.0f;

            vertices[2].position[0] = (float)position[0] + 1.0f;
            vertices[2].position[1] = (float)position[1] + 1.0f;
            vertices[2].position[2] = (float)position[2] + 1.0f;

            vertices[3].position[0] = (float)position[0] + 1.0f;
            vertices[3].position[1] = (float)position[1] + 1.0f;
            vertices[3].position[2] = (float)position[2];

            for (i32 i = 0; i < 4; i++) {
                vertices[i].normal[0] = 0.0f;
                vertices[i].normal[1] = 1.0f;
                vertices[i].normal[2] = 0.0f;
            }
            break;
        case BLOCK_FACE_FRONT:
            vertices[0].position[0] = (float)position[0];
            vertices[0].position[1] = (float)position[1];
            vertices[0].position[2] = (float)position[2];

            vertices[1].position[0] = (float)position[0];
            vertices[1].position[1] = (float)position[1] + 1.0f;
            vertices[1].position[2] = (float)position[2];

            vertices[2].position[0] = (float)position[0] + 1.0f;
            vertices[2].position[1] = (float)position[1] + 1.0f;
            vertices[2].position[2] = (float)position[2];

            vertices[3].position[0] = (float)position[0] + 1.0f;
            vertices[3].position[1] = (float)position[1];
            vertices[3].position[2] = (float)position[2];

            for (i32 i = 0; i < 4; i++) {
                vertices[i].normal[0] = 0.0f;
                vertices[i].normal[1] = 0.0f;
                vertices[i].normal[2] = -1.0f;
            }
            break;
        case BLOCK_FACE_BACK:
            vertices[0].position[0] = (float)position[0] + 1.0f;
            vertices[0].position[1] = (float)position[1];
            vertices[0].position[2] = (float)position[2] + 1.0f;

            vertices[1].position[0] = (float)position[0] + 1.0f;
            vertices[1].position[1] = (float)position[1] + 1.0f;
            vertices[1].position[2] = (float)position[2] + 1.0f;

            vertices[2].position[0] = (float)position[0];
            vertices[2].position[1] = (float)position[1] + 1.0f;
            vertices[2].position[2] = (float)position[2] + 1.0f;

            vertices[3].position[0] = (float)position[0];
            vertices[3].position[1] = (float)position[1];
            vertices[3].position[2] = (float)position[2] + 1.0f;

            for (i32 i = 0; i < 4; i++) {
                vertices[i].normal[0] = 0.0f;
                vertices[i].normal[1] = 0.0f;
                vertices[i].normal[2] = 1.0f;
            }
            break;
    }

    for (i32 i = 0; i < 4; i++) {
        if (face == BLOCK_FACE_TOP && (flags & BLOCK_FLAG_TEXTURE_TOP)) {
            ivec2 atlas_pos = { BLOCK_ID_TO_ATLAS_POS_TOP(block->id) };
            vec2 uv = ATLAS_TEXTURE_SLOT_UV(atlas_pos[0], atlas_pos[1]);

            vertices[i].uv[0] = uv[0] + (float)(i == 1 || i == 2) / ATLAS_TEXTURE_SLOT_COUNT;
            vertices[i].uv[1] = uv[1] + (float)(i == 2 || i == 3) / ATLAS_TEXTURE_SLOT_COUNT;
        } else if (face == BLOCK_FACE_BOTTOM && (flags & BLOCK_FLAG_TEXTURE_BOTTOM)) {
            ivec2 atlas_pos = { BLOCK_ID_TO_ATLAS_POS_BOTTOM(block->id) };
            vec2 uv = ATLAS_TEXTURE_SLOT_UV(atlas_pos[0], atlas_pos[1]);

            vertices[i].uv[0] = uv[0] + (float)(i == 1 || i == 2) / ATLAS_TEXTURE_SLOT_COUNT;
            vertices[i].uv[1] = uv[1] + (float)(i == 2 || i == 3) / ATLAS_TEXTURE_SLOT_COUNT;
        } else {
            ivec2 atlas_pos = { BLOCK_ID_TO_ATLAS_POS(block->id) };
            vec2 uv = ATLAS_TEXTURE_SLOT_UV(atlas_pos[0], atlas_pos[1]);

            vertices[i].uv[0] = uv[0] + (float)(i == 0 || i == 1) / ATLAS_TEXTURE_SLOT_COUNT;
            vertices[i].uv[1] = uv[1] + (float)(i == 0 || i == 3) / ATLAS_TEXTURE_SLOT_COUNT;
        }
    }

    u32 indices[6] = { mesh->vertex_count + 0, mesh->vertex_count + 1, mesh->vertex_count + 2,
                       mesh->vertex_count + 2, mesh->vertex_count + 3, mesh->vertex_count + 0 };

    memcpy(&mesh->vertices[mesh->vertex_count], vertices, sizeof(vertices));
    memcpy(&mesh->indices[mesh->index_count], indices, sizeof(indices));

    mesh->vertex_count += 4;
    mesh->index_count += 6;
}

void chunk_draw(chunk_t* chunk) {
    if (chunk->mesh.vertex_count == 0) {
        return;
    }

    mat4 model = { 0 };
    glm_mat4_identity(model);
    glm_translate(
        model,
        (vec3){ (float)chunk->position[0] * CHUNK_SIZE,
                (float)chunk->position[1] * CHUNK_SIZE,
                (float)chunk->position[2] * CHUNK_SIZE }
    );

    shader_set_mat4(&g_shader_current, "u_model", model);

    mesh_draw(&chunk->mesh);
}

world_t* world_new(void) {
    world_t* world = malloc(sizeof(world_t));
    memset(world, 0, sizeof(world_t));

    world->chunks = malloc(sizeof(chunk_t) * MAX_LOADED_CHUNKS);
    world->loaded_chunk_count = 0;

    memset(world->chunk_slot_bitmap, 0, sizeof(world->chunk_slot_bitmap));
    memset(world->chunk_slot_remeshed_bitmap, 0, sizeof(world->chunk_slot_remeshed_bitmap));
    memset(world->chunk_slot_remesh_queue, 0, sizeof(world->chunk_slot_remesh_queue));
    world->chunk_slot_remesh_queue_count = 0;

    return world;
}

bool world_chunk_slot_is_taken(world_t* world, u32 index) {
    return world->chunk_slot_bitmap[index / 8] & (1 << (index % 8));
}

void world_chunk_slot_set_taken(world_t* world, u32 index) {
    world->chunk_slot_bitmap[index / 8] |= (1 << (index % 8));
}

void world_chunk_slot_set_free(world_t* world, u32 index) {
    world->chunk_slot_bitmap[index / 8] &= (u8) ~(1 << (index % 8));
}

bool world_chunk_slot_is_remeshed(world_t* world, u32 index) {
    return world->chunk_slot_remeshed_bitmap[index / 8] & (1 << (index % 8));
}

void world_chunk_slot_set_remeshed(world_t* world, u32 index) {
    world->chunk_slot_remeshed_bitmap[index / 8] |= (1 << (index % 8));
}

void world_chunk_slot_set_unremeshed(world_t* world, u32 index) {
    world->chunk_slot_remeshed_bitmap[index / 8] &= (u8) ~(1 << (index % 8));
}

void world_chunk_slot_clear_remeshed(world_t* world) {
    memset(world->chunk_slot_remeshed_bitmap, 0, sizeof(world->chunk_slot_remeshed_bitmap));
}

void world_free(world_t* world) {
    for (u32 i = 0; i < MAX_LOADED_CHUNKS; i++) {
        if (!world_chunk_slot_is_taken(world, i)) {
            continue;
        }

        chunk_forget(&world->chunks[i]);
    }
    free(world->chunks);
    free(world);
}

void world_remesh_queue_add(world_t* world, u32 index) {
    world->chunk_slot_remesh_queue[world->chunk_slot_remesh_queue_count++] = index;
}

void world_remesh_queue_clear(world_t* world) {
    world->chunk_slot_remesh_queue_count = 0;
}

void world_remesh_queue_process(world_t* world) {
    if (world->chunk_slot_remesh_queue_count == 0) {
        return;
    }

    world_chunk_slot_clear_remeshed(world);
    for (u32 i = 0; i < world->chunk_slot_remesh_queue_count; i++) {
        if (!world_chunk_slot_is_taken(world, world->chunk_slot_remesh_queue[i])) {
            continue;
        }
        if (world_chunk_slot_is_remeshed(world, world->chunk_slot_remesh_queue[i])) {
            continue;
        }
        chunk_t* chunk = &world->chunks[world->chunk_slot_remesh_queue[i]];
        chunk_remesh(chunk, world);
    }
    world_remesh_queue_clear(world);
}

chunk_t* world_get_chunk(world_t* world, ivec3 position) {
    for (u32 i = 0; i < MAX_LOADED_CHUNKS; i++) {
        if (!world_chunk_slot_is_taken(world, i)) {
            continue;
        }
        if (glme_ivec3_eq(world->chunks[i].position, position)) {
            return &world->chunks[i];
        }
    }
    return NULL;
}

chunk_t* world_get_or_load_chunk(world_t* world, ivec3 position) {
    chunk_t* chunk = world_get_chunk(world, position);
    if (chunk) {
        return chunk;
    }

    chunk = world_get_chunk_slot(world);
    world_chunk_slot_set_taken(world, (u32)(chunk - world->chunks));
    chunk_init(chunk, world, position);

    const ivec3 NEIGHBOR_OFFSETS[6] = {
        { 0, 0, -1 }, { 0, 0, 1 }, { 0, -1, 0 }, { 0, 1, 0 }, { -1, 0, 0 }, { 1, 0, 0 },
    };

    for (u32 i = 0; i < 6; i++) {
        chunk_t* neighbor = world_get_chunk(
            world,
            (ivec3){
                position[0] + NEIGHBOR_OFFSETS[i][0],
                position[1] + NEIGHBOR_OFFSETS[i][1],
                position[2] + NEIGHBOR_OFFSETS[i][2],
            }
        );

        if (neighbor) {
            world_remesh_queue_add(world, (u32)(neighbor - world->chunks));
        }
    }

    return chunk;
}

chunk_t* world_get_chunk_slot(world_t* world) {
    if (world->loaded_chunk_count >= MAX_LOADED_CHUNKS) {
        float distance = 0.0f;
        u32 chunk_to_unload = 0;
        for (u32 i = 0; i < MAX_LOADED_CHUNKS; i++) {
            // world is full = no need to check if slot is taken

            vec3 diff;
            glm_vec3_sub(
                (vec3){ (float)world->chunks[i].position[0] * CHUNK_SIZE,
                        (float)world->chunks[i].position[1] * CHUNK_SIZE,
                        (float)world->chunks[i].position[2] * CHUNK_SIZE },
                g_player.position,
                diff
            );
            float d = glm_vec3_norm(diff);

            if (d > distance) {
                distance = d;
                chunk_to_unload = i;
            }
        }

        chunk_forget(&world->chunks[chunk_to_unload]);
        world_chunk_slot_set_free(world, chunk_to_unload);

        return &world->chunks[chunk_to_unload];
    }

    return &world->chunks[world->loaded_chunk_count++];
}

void world_unload_chunk(world_t* world, ivec3 position) {
    for (u32 i = 0; i < MAX_LOADED_CHUNKS; i++) {
        if (!world_chunk_slot_is_taken(world, i)) {
            continue;
        }

        if (glme_ivec3_eq(world->chunks[i].position, position)) {
            chunk_forget(&world->chunks[i]);
            world_chunk_slot_set_free(world, i);

            world->loaded_chunk_count--;
            return;
        }
    }
}

void world_unload_all_chunks(world_t* world) {
    for (u32 i = 0; i < MAX_LOADED_CHUNKS; i++) {
        if (!world_chunk_slot_is_taken(world, i)) {
            continue;
        }

        chunk_forget(&world->chunks[i]);
        world_chunk_slot_set_free(world, i);
    }
    world->loaded_chunk_count = 0;
}

void world_draw(world_t* world) {
    for (u32 i = 0; i < MAX_LOADED_CHUNKS; i++) {
        if (!world_chunk_slot_is_taken(world, i)) {
            continue;
        }
        chunk_draw(&world->chunks[i]);
    }
}

void world_get_chunk_position(ivec3 position, ivec3 chunk_position) {
    chunk_position[0] =
        position[0] >= 0 ? position[0] / CHUNK_SIZE : (position[0] + 1) / CHUNK_SIZE - 1;
    chunk_position[1] =
        position[1] >= 0 ? position[1] / CHUNK_SIZE : (position[1] + 1) / CHUNK_SIZE - 1;
    chunk_position[2] =
        position[2] >= 0 ? position[2] / CHUNK_SIZE : (position[2] + 1) / CHUNK_SIZE - 1;
}

void world_get_chunk_positionf(vec3 position, ivec3 chunk_position) {
    chunk_position[0] = position[0] >= 0 ? (i32)position[0] / CHUNK_SIZE
                                         : ((i32)position[0] + 1) / CHUNK_SIZE - 1;
    chunk_position[1] = position[1] >= 0 ? (i32)position[1] / CHUNK_SIZE
                                         : ((i32)position[1] + 1) / CHUNK_SIZE - 1;
    chunk_position[2] = position[2] >= 0 ? (i32)position[2] / CHUNK_SIZE
                                         : ((i32)position[2] + 1) / CHUNK_SIZE - 1;
}

void world_get_position_in_chunk(ivec3 position, ivec3 position_in_chunk) {
    position_in_chunk[0] = posmod(position[0], CHUNK_SIZE);
    position_in_chunk[1] = posmod(position[1], CHUNK_SIZE);
    position_in_chunk[2] = posmod(position[2], CHUNK_SIZE);
}

block_t* world_get_block_at(world_t* world, ivec3 position) {
    ivec3 chunk_position = { 0 };

    world_get_chunk_position(position, chunk_position);

    chunk_t* chunk = world_get_chunk(world, chunk_position);
    if (!chunk) {
        return NULL;
    }

    ivec3 block_position_in_chunk;

    world_get_position_in_chunk(position, block_position_in_chunk);

    return &chunk->blocks[CHUNK_POS_TO_INDEX(
        block_position_in_chunk[0],
        block_position_in_chunk[1],
        block_position_in_chunk[2]
    )];
}

void world_set_block_at(world_t* world, ivec3 position, block_id_t block) {
    ivec3 chunk_position = { 0 };

    world_get_chunk_position(position, chunk_position);

    chunk_t* chunk = world_get_or_load_chunk(world, chunk_position);

    ivec3 block_position;

    world_get_position_in_chunk(position, block_position);

    chunk_set_block(chunk, world, block_position, block);
}

void world_try_set_block_at(world_t* world, ivec3 position, block_id_t block) {
    ivec3 chunk_position = { 0 };

    world_get_chunk_position(position, chunk_position);

    chunk_t* chunk = world_get_chunk(world, chunk_position);

    if (!chunk) {
        return;
    }

    ivec3 block_position;

    world_get_position_in_chunk(position, block_position);

    chunk_set_block(chunk, world, block_position, block);

    if (block_position[0] == 0) {
        ivec3 neighbor_position = { chunk_position[0] - 1,
                                    chunk_position[1],
                                    chunk_position[2] };
        chunk_t* neighbor = world_get_chunk(world, neighbor_position);
        if (neighbor) {
            world_remesh_queue_add(world, (u32)(neighbor - world->chunks));
        }
    } else if (block_position[0] == CHUNK_SIZE - 1) {
        ivec3 neighbor_position = { chunk_position[0] + 1,
                                    chunk_position[1],
                                    chunk_position[2] };
        chunk_t* neighbor = world_get_chunk(world, neighbor_position);
        if (neighbor) {
            world_remesh_queue_add(world, (u32)(neighbor - world->chunks));
        }
    }

    if (block_position[1] == 0) {
        ivec3 neighbor_position = { chunk_position[0],
                                    chunk_position[1] - 1,
                                    chunk_position[2] };
        chunk_t* neighbor = world_get_chunk(world, neighbor_position);
        if (neighbor) {
            world_remesh_queue_add(world, (u32)(neighbor - world->chunks));
        }
    } else if (block_position[1] == CHUNK_SIZE - 1) {
        ivec3 neighbor_position = { chunk_position[0],
                                    chunk_position[1] + 1,
                                    chunk_position[2] };
        chunk_t* neighbor = world_get_chunk(world, neighbor_position);
        if (neighbor) {
            world_remesh_queue_add(world, (u32)(neighbor - world->chunks));
        }
    }

    if (block_position[2] == 0) {
        ivec3 neighbor_position = { chunk_position[0],
                                    chunk_position[1],
                                    chunk_position[2] - 1 };
        chunk_t* neighbor = world_get_chunk(world, neighbor_position);
        if (neighbor) {
            world_remesh_queue_add(world, (u32)(neighbor - world->chunks));
        }
    } else if (block_position[2] == CHUNK_SIZE - 1) {
        ivec3 neighbor_position = { chunk_position[0],
                                    chunk_position[1],
                                    chunk_position[2] + 1 };
        chunk_t* neighbor = world_get_chunk(world, neighbor_position);
        if (neighbor) {
            world_remesh_queue_add(world, (u32)(neighbor - world->chunks));
        }
    }
}
