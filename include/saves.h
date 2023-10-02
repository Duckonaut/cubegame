#pragma once

#include <string.h>
#include <stdio.h>

#include "types.h"
#include "world.h"

#define SAVE_MAGIC_STR "CGSV"
#define SAVE_MAGIC_LEN 4

typedef struct world_save_chunk {
    i32 x;
    i32 y;
    i32 z;
    u32 _pad;

    block_id_t block_data[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
} world_save_chunk_t;

typedef struct world_save {
    u64 seed;
    u32 chunk_count;

    world_save_chunk_t* chunks;
} world_save_t;

typedef struct save_header {
    u32 magic;
    u32 save_format_version;
} save_header_t;

typedef struct save {
    save_header_t header;
    world_save_t world;
} save_t;

extern save_t* g_save;

save_t* save_new();
save_t* save_load(const char* path);
void save_free(save_t* save);

void save_write(const save_t* save, const char* path);

void save_write_world(const world_save_t* world, FILE* file);
void save_write_chunk(const world_save_chunk_t* chunk, FILE* file);

bool save_read_world(FILE* file, world_save_t* world);
bool save_read_chunk(FILE* file, world_save_chunk_t* chunk);

void save_add_chunk(save_t* save, const chunk_t* chunk);
