#pragma once

#include "mesh.h"
#include "types.h"

#include <cglm/types.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

#define CHUNK_SIZE 16
#define CHUNK_BLOCK_COUNT (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

#define BLOCK_AIR (block_id_t)0
#define CHUNK_POS_TO_INDEX(x, y, z) ((x) + (y)*CHUNK_SIZE + (z)*CHUNK_SIZE * CHUNK_SIZE)
#define CHUNK_INDEX_TO_IVEC(index)                                                             \
    (ivec3) {                                                                                  \
        (index) % CHUNK_SIZE, ((index) / CHUNK_SIZE) % CHUNK_SIZE,                             \
            (index) / (CHUNK_SIZE * CHUNK_SIZE)                                                \
    }

#define WORLD_VISIBLE_CHUNK_RADIUS 2

#define BLOCK_ID_MAX 6
#define BLOCK_ID_TO_ATLAS_POS(id)                                                              \
    ((id) % ATLAS_TEXTURE_SLOT_COUNT), (((id) / ATLAS_TEXTURE_SLOT_COUNT) * 3)
#define BLOCK_ID_TO_ATLAS_POS_TOP(id)                                                          \
    ((id) % ATLAS_TEXTURE_SLOT_COUNT), (((id) / ATLAS_TEXTURE_SLOT_COUNT) * 3 +                \
                                        (block_flags[id] & BLOCK_FLAG_TEXTURE_TOP ? 1 : 0))
#define BLOCK_ID_TO_ATLAS_POS_BOTTOM(id)                                                       \
    ((id) % ATLAS_TEXTURE_SLOT_COUNT), (((id) / ATLAS_TEXTURE_SLOT_COUNT) * 3 +                \
                                        (block_flags[id] & BLOCK_FLAG_TEXTURE_BOTTOM ? 2 : 0))

typedef u8 block_id_t;
typedef enum block_face {
    BLOCK_FACE_LEFT = 0,
    BLOCK_FACE_RIGHT = 1,
    BLOCK_FACE_BOTTOM = 2,
    BLOCK_FACE_TOP = 3,
    BLOCK_FACE_FRONT = 4,
    BLOCK_FACE_BACK = 5,
} block_face_t;

typedef enum block_flags {
    // Block is solid and cannot be walked through
    BLOCK_FLAG_SOLID = 1 << 0,
    // Block is transparent, treat as air when meshing nearby blocks
    BLOCK_FLAG_TRANSPARENT = 1 << 1,
    // Block is meshed, do not mesh nearby blocks
    BLOCK_FLAG_MESHED = 1 << 2,
    // Block is destructible, can be broken
    BLOCK_FLAG_DESTRUCTIBLE = 1 << 3,
    // Block has a custom texture on the top face.
    // Will take the texture one cell down
    BLOCK_FLAG_TEXTURE_TOP = 1 << 4,
    // Block has a custom texture on the bottom face.
    // Will take the texture two cells down
    BLOCK_FLAG_TEXTURE_BOTTOM = 1 << 5,
} block_flags_t;

extern block_flags_t block_flags[BLOCK_ID_MAX];

typedef struct block {
    block_id_t id;
    u32 mesh_vertex_offset;
    u32 mesh_vertex_count;
    i32 mesh_index_offset;
    i32 mesh_index_count;
} block_t;

void block_mesh_face(mesh_t* mesh, ivec3 position, block_face_t face, block_t block);

typedef struct chunk {
    ivec3 position;
    mesh_t mesh;
    block_t blocks[CHUNK_BLOCK_COUNT];
} chunk_t;

// Initialize a chunk in place
void chunk_init(chunk_t* chunk, ivec3 position);
// Destroy a chunk in place, freeing all memory associated with it
// Does not free the chunk itself
void chunk_forget(chunk_t* chunk);

// Generate chunk blocks
// Does not mesh the chunk
void chunk_generate(chunk_t* chunk);

// Can be used to remove a block from a chunk, ie. set it to air
void chunk_set_block(chunk_t* chunk, ivec3 position, block_id_t id);
block_t chunk_get_block(chunk_t* chunk, ivec3 position);

// Mesh a chunk in place
// Does not free the chunk's previous mesh
void chunk_mesh(chunk_t* chunk);
// Destroy a chunk's mesh in place, freeing all memory associated with it
// Does not free the chunk itself or its blocks
void chunk_forget_mesh(chunk_t* chunk);

void chunk_remesh(chunk_t* chunk);
void chunk_remesh_block(chunk_t* chunk, ivec3 position);

void chunk_draw(chunk_t* chunk);

#define MAX_LOADED_CHUNKS 256 // 8 in X, 4 in Y, 8 in Z

typedef struct world {
    chunk_t* chunks;
    u32 loaded_chunk_count;
} world_t;

// Create a new world
// World must be freed with world_free
// World is always allocated on the heap, too large to fit on the stack
world_t* world_new(void);
void world_free(world_t* world);

// Get a chunk from the world
// If the chunk is not loaded, NULL is returned
chunk_t* world_get_chunk(world_t* world, ivec3 position);
// Get a chunk from the world, loading it if it is not already loaded
chunk_t* world_get_or_load_chunk(world_t* world, ivec3 position);
// Get a chunk slot from the world
// If the chunks array can fit the chunk, loaded_chunk_count is incremented
// If the chunks array cannot fit the chunk, the most-useless chunk is unloaded
chunk_t* world_get_chunk_slot(world_t* world);

block_t* world_get_block_at(world_t* world, ivec3 position);
void world_set_block_at(world_t* world, ivec3 position, block_id_t id);
void world_try_set_block_at(world_t* world, ivec3 position, block_id_t id);

// Unload a chunk from the world
void world_unload_chunk(world_t* world, ivec3 position);
// Unload all chunks from the world
// Does not free the world itself
void world_unload_all_chunks(world_t* world);

void world_draw(world_t* world);
