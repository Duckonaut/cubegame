#include "saves.h"
#include "config.h"
#include "log.h"
#include "types.h"
#include "world.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

save_t* g_save;

save_t* save_new() {
    save_t* save = malloc(sizeof(save_t));

    memcpy(&save->header.magic, SAVE_MAGIC_STR, 4);
    save->header.save_format_version = SAVE_FORMAT_VERSION;

    save->world.seed = 0;
    save->world.chunk_count = 0;
    save->world.chunks = NULL;

    return save;
}

save_t* save_load(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }

    save_t* save = malloc(sizeof(save_t));

    save_header_t header;
    fread(&header, sizeof(save_header_t), 1, file);

    if (memcmp(&header.magic, SAVE_MAGIC_STR, 4) != 0) {
        fclose(file);
        return NULL;
    }

    if (header.save_format_version != SAVE_FORMAT_VERSION) {
        fclose(file);
        return NULL;
    }

    save->header = header;

    bool result = save_read_world(file, &save->world);

    if (!result) {
        fclose(file);
        free(save);
        return NULL;
    }

    fclose(file);

    return save;
}

void save_free(save_t* save) {
    if (save->world.chunks != NULL) {
        free(save->world.chunks);
    }

    free(save);
}

void save_write(const save_t* save, const char* path) {
    FILE* file = fopen(path, "wb");
    if (!file) {
        return;
    }

    fwrite(&save->header, sizeof(save_header_t), 1, file);

    save_write_world(&save->world, file);

    fclose(file);

    return;
}

void save_write_world(const world_save_t* world, FILE* file) {
    fwrite(&world->seed, sizeof(u64), 1, file);
    fwrite(&world->chunk_count, sizeof(u32), 1, file);

    for (size_t i = 0; i < world->chunk_count; i++) {
        save_write_chunk(&world->chunks[i], file);
    }
}

void save_write_chunk(const world_save_chunk_t* chunk, FILE* file) {
    fwrite(&chunk->x, sizeof(i32), 1, file);
    fwrite(&chunk->y, sizeof(i32), 1, file);
    fwrite(&chunk->z, sizeof(i32), 1, file);

    fwrite(
        &chunk->block_data,
        sizeof(block_id_t),
        CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE,
        file
    );
}

bool save_read_world(FILE* file, world_save_t* world) {
    if (!fread(&world->seed, sizeof(u64), 1, file)) {
        return false;
    }

    if (!fread(&world->chunk_count, sizeof(u32), 1, file)) {
        return false;
    }

    world->chunks = malloc(sizeof(world_save_chunk_t) * world->chunk_count);

    for (size_t i = 0; i < world->chunk_count; i++) {
        if (!save_read_chunk(file, &world->chunks[i])) {
            free(world->chunks);
            return false;
        }
    }

    return true;
}

bool save_read_chunk(FILE* file, world_save_chunk_t* chunk) {
    if (!fread(&chunk->x, sizeof(i32), 1, file)) {
        return false;
    }

    if (!fread(&chunk->y, sizeof(i32), 1, file)) {
        return false;
    }

    if (!fread(&chunk->z, sizeof(i32), 1, file)) {
        return false;
    }

    if (!fread(
            &chunk->block_data,
            sizeof(block_id_t),
            CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE,
            file
        )) {
        return false;
    }

    return true;
}

void save_add_chunk(save_t* save, const chunk_t* chunk) {
    world_save_chunk_t* world_chunk = NULL;

    for (size_t i = 0; i < save->world.chunk_count; i++) {
        if (save->world.chunks[i].x == chunk->position[0] &&
            save->world.chunks[i].y == chunk->position[1] &&
            save->world.chunks[i].z == chunk->position[2]) {
            world_chunk = &save->world.chunks[i];
            break;
        }
    }

    bool is_new_chunk = world_chunk == NULL;

    if (is_new_chunk) {
        LOG_INFO("Adding new chunk to save\n");
        save->world.chunk_count++;

        LOG_INFO("New chunk count: %d\n", save->world.chunk_count);

        save->world.chunks =
            realloc(save->world.chunks, sizeof(world_save_chunk_t) * save->world.chunk_count);

        if (!save->world.chunks) {
            LOG_ERROR("Failed to allocate memory for new chunk\n");
            return;
        }

        world_chunk = &save->world.chunks[save->world.chunk_count - 1];

        world_chunk->x = chunk->position[0];
        world_chunk->y = chunk->position[1];
        world_chunk->z = chunk->position[2];
    } else {
        LOG_INFO("Updating existing chunk in save\n");
    }

    for (size_t z = 0; z < CHUNK_SIZE; z++) {
        for (size_t x = 0; x < CHUNK_SIZE; x++) {
            for (size_t y = 0; y < CHUNK_SIZE; y++) {
                size_t index = z * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + x;
                world_chunk->block_data[index] = chunk->blocks[index].id;
            }
        }
    }
}
