#include "world.h"
#include "assets.h"
#include "log.h"
#include "mesh.h"
#include "shader.h"
#include <cglm/affine-pre.h>
#include <cglm/ivec3.h>
#include <cglm/mat4.h>
#include <cglm/types.h>
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
    (block_flags_t
    )(BLOCK_FLAG_SOLID | BLOCK_FLAG_MESHED | BLOCK_FLAG_TEXTURE_TOP | BLOCK_FLAG_TEXTURE_BOTTOM
    ),
    // BLOCK_ID_BRICK
    (block_flags_t)(BLOCK_FLAG_SOLID | BLOCK_FLAG_MESHED),
    // BLOCK_ID_GLASS
    (block_flags_t)(BLOCK_FLAG_SOLID | BLOCK_FLAG_MESHED | BLOCK_FLAG_TRANSPARENT),
};

void chunk_init(chunk_t* chunk, ivec3 position) {
    glm_ivec3_copy(position, chunk->position);
    for (int i = 0; i < CHUNK_SIZE; i++) {
        chunk->blocks[i] = (block_t){ .id = BLOCK_AIR };
    }
    chunk->mesh.vertices = NULL;
    chunk->mesh.indices = NULL;

    chunk_generate(chunk);
    chunk_mesh(chunk);
}
void chunk_forget(chunk_t* chunk) {
    chunk_forget_mesh(chunk);
    free(chunk->mesh.vertices);
    free(chunk->mesh.indices);
}

void chunk_generate(chunk_t* chunk) {
    for (i32 x = 0; x < CHUNK_SIZE; x++) {
        for (i32 z = 0; z < CHUNK_SIZE; z++) {
            i32 height = 30;
            i32 rock_height = 20;

            for (i32 y = 0; y < CHUNK_SIZE; y++) {
                i32 index = CHUNK_POS_TO_INDEX(x, y, z);
                i32 world_y = y + chunk->position[1] * CHUNK_SIZE;
                if (world_y == height) {
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

void chunk_set_block(chunk_t* chunk, ivec3 position, block_id_t id) {
    i32 index = CHUNK_POS_TO_INDEX(position[0], position[1], position[2]);
    chunk->blocks[index].id = id;
    chunk_remesh(chunk);
}

block_t chunk_get_block(chunk_t* chunk, ivec3 position) {
    i32 index = CHUNK_POS_TO_INDEX(position[0], position[1], position[2]);
    return chunk->blocks[index];
}

void chunk_mesh(chunk_t* chunk) {
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

    for (i32 x = 0; x < CHUNK_SIZE; x++) {
        for (i32 y = 0; y < CHUNK_SIZE; y++) {
            for (i32 z = 0; z < CHUNK_SIZE; z++) {
                chunk_remesh_block(chunk, (ivec3){ x, y, z });
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

void chunk_remesh(chunk_t* chunk) {
    chunk_forget_mesh(chunk);
    chunk_mesh(chunk);
}

void chunk_remesh_block(chunk_t* chunk, ivec3 pos) {
    block_t block = chunk_get_block(chunk, pos);
    if (block.id == BLOCK_AIR) {
        return;
    }

    block_flags_t flags = block_flags[block.id];
    if (!(flags & BLOCK_FLAG_MESHED)) {
        return;
    }

    ivec3 block_pos = { 0 };
    // Ignore other chunks for now
    glm_ivec3_copy(pos, block_pos);

    static ivec3 DIRECTION_OFFSETS[6] = {
        { -1, 0, 0 }, { 1, 0, 0 }, { 0, -1, 0 }, { 0, 1, 0 }, { 0, 0, -1 }, { 0, 0, 1 },
    };

    block.mesh_index_offset = chunk->mesh.index_count;
    block.mesh_vertex_offset = chunk->mesh.vertex_count;

    for (i32 i = 0; i < 6; i++) {
        ivec3 neighbor_pos = { 0 };
        glm_ivec3_add(block_pos, DIRECTION_OFFSETS[i], neighbor_pos);

        if (neighbor_pos[0] < 0 || neighbor_pos[0] >= CHUNK_SIZE || neighbor_pos[1] < 0 ||
            neighbor_pos[1] >= CHUNK_SIZE || neighbor_pos[2] < 0 ||
            neighbor_pos[2] >= CHUNK_SIZE) {
            block_mesh_face(&chunk->mesh, block_pos, (block_face_t)i, block);
            continue;
        }

        block_t neighbor = chunk_get_block(chunk, neighbor_pos);
        if (neighbor.id == BLOCK_AIR || (block_flags[neighbor.id] & BLOCK_FLAG_TRANSPARENT)) {
            block_mesh_face(&chunk->mesh, block_pos, (block_face_t)i, block);
        }
    }

    block.mesh_index_count = chunk->mesh.index_count - block.mesh_index_offset;
    block.mesh_vertex_count = chunk->mesh.vertex_count - block.mesh_vertex_offset;
}

void block_mesh_face(mesh_t* mesh, ivec3 position, block_face_t face, block_t block) {
    block_flags_t flags = block_flags[block.id];

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
            break;
    }

    for (i32 i = 0; i < 4; i++) {
        vertices->color[0] = vertices->position[0] > (float)position[0];
        vertices->color[1] = vertices->position[1] > (float)position[1];
        vertices->color[2] = vertices->position[2] > (float)position[2];

        if (face == BLOCK_FACE_TOP && (flags & BLOCK_FLAG_TEXTURE_TOP)) {
            ivec2 atlas_pos = { BLOCK_ID_TO_ATLAS_POS_TOP(block.id) };
            vec2 uv = ATLAS_TEXTURE_SLOT_UV(atlas_pos[0], atlas_pos[1]);

            vertices[i].uv[0] = uv[0] + (float)(i == 1 || i == 2) / ATLAS_TEXTURE_SLOT_COUNT;
            vertices[i].uv[1] = uv[1] + (float)(i == 2 || i == 3) / ATLAS_TEXTURE_SLOT_COUNT;
        } else if (face == BLOCK_FACE_BOTTOM && (flags & BLOCK_FLAG_TEXTURE_BOTTOM)) {
            ivec2 atlas_pos = { BLOCK_ID_TO_ATLAS_POS_BOTTOM(block.id) };
            vec2 uv = ATLAS_TEXTURE_SLOT_UV(atlas_pos[0], atlas_pos[1]);

            vertices[i].uv[0] = uv[0] + (float)(i == 1 || i == 2) / ATLAS_TEXTURE_SLOT_COUNT;
            vertices[i].uv[1] = uv[1] + (float)(i == 2 || i == 3) / ATLAS_TEXTURE_SLOT_COUNT;
        } else {
            ivec2 atlas_pos = { BLOCK_ID_TO_ATLAS_POS(block.id) };
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

    return world;
}

void world_free(world_t* world) {
    for (u32 i = 0; i < world->loaded_chunk_count; i++) {
        chunk_forget(&world->chunks[i]);
    }
    free(world->chunks);
    free(world);
}

chunk_t* world_get_chunk(world_t* world, ivec3 position) {
    for (u32 i = 0; i < world->loaded_chunk_count; i++) {
        if (world->chunks[i].position[0] == position[0] &&
            world->chunks[i].position[1] == position[1] &&
            world->chunks[i].position[2] == position[2]) {
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

    if (world->loaded_chunk_count >= MAX_LOADED_CHUNKS) {
        chunk = world_get_chunk_slot(world);
        chunk_init(chunk, position);
        return chunk;
    }

    chunk = &world->chunks[world->loaded_chunk_count++];
    chunk_init(chunk, position);

    return chunk;
}

chunk_t* world_get_chunk_slot(world_t* world) {
    if (world->loaded_chunk_count >= MAX_LOADED_CHUNKS) {
        chunk_forget(&world->chunks[0]);
        // TODO: Probably not the best way to do this
        memmove(
            &world->chunks[0],
            &world->chunks[1],
            sizeof(chunk_t*) * (MAX_LOADED_CHUNKS - 1)
        );
        return &world->chunks[MAX_LOADED_CHUNKS - 1];
    }

    return &world->chunks[world->loaded_chunk_count++];
}

void world_unload_chunk(world_t* world, ivec3 position) {
    for (u32 i = 0; i < world->loaded_chunk_count; i++) {
        if (world->chunks[i].position[0] == position[0] &&
            world->chunks[i].position[1] == position[1] &&
            world->chunks[i].position[2] == position[2]) {
            chunk_forget(&world->chunks[i]);
            memmove(
                &world->chunks[i],
                &world->chunks[i + 1],
                sizeof(chunk_t*) * (world->loaded_chunk_count - i - 1)
            );
            world->loaded_chunk_count--;
            return;
        }
    }
}

void world_unload_all_chunks(world_t* world) {
    for (u32 i = 0; i < world->loaded_chunk_count; i++) {
        chunk_forget(&world->chunks[i]);
    }
    world->loaded_chunk_count = 0;
}

void world_draw(world_t* world) {
    for (u32 i = 0; i < world->loaded_chunk_count; i++) {
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

block_t* world_get_block_at(world_t* world, ivec3 position) {
    ivec3 chunk_position = { 0 };

    world_get_chunk_position(position, chunk_position);

    chunk_t* chunk = world_get_chunk(world, chunk_position);
    if (!chunk) {
        return NULL;
    }

    ivec3 block_position = { position[0] % CHUNK_SIZE,
                             position[1] % CHUNK_SIZE,
                             position[2] % CHUNK_SIZE };

    return &chunk->blocks
                [CHUNK_POS_TO_INDEX(block_position[0], block_position[1], block_position[2])];
}

void world_set_block_at(world_t* world, ivec3 position, block_id_t block) {
    ivec3 chunk_position = { 0 };

    world_get_chunk_position(position, chunk_position);

    chunk_t* chunk = world_get_or_load_chunk(world, chunk_position);

    ivec3 block_position = { position[0] % CHUNK_SIZE,
                             position[1] % CHUNK_SIZE,
                             position[2] % CHUNK_SIZE };

    chunk_set_block(chunk, block_position, block);
}

void world_try_set_block_at(world_t* world, ivec3 position, block_id_t block) {
    ivec3 chunk_position = { 0 };

    world_get_chunk_position(position, chunk_position);

    chunk_t* chunk = world_get_chunk(world, chunk_position);

    if (!chunk) {
        return;
    }

    ivec3 block_position = { position[0] % CHUNK_SIZE,
                             position[1] % CHUNK_SIZE,
                             position[2] % CHUNK_SIZE };

    chunk_set_block(chunk, block_position, block);
}
