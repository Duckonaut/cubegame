#pragma once

#include "types.h"

#define ASSET_PATH "assets/"

u8* read_file(const char* path, usize* size);
u8* read_asset(const char* path, usize* size);

void free_file(u8* file);

#define ATLAS_TEXTURE_SLOT_SIZE 16
#define ATLAS_TEXTURE_SLOT_COUNT 16
#define ATLAS_TEXTURE_SIZE (ATLAS_TEXTURE_SLOT_SIZE * ATLAS_TEXTURE_SLOT_SIZE)

#define ATLAS_TEXTURE_SLOT_UV(x, y)                                                            \
    { (f32)(x) / (f32)ATLAS_TEXTURE_SLOT_COUNT, (f32)(y) / (f32)ATLAS_TEXTURE_SLOT_COUNT }

#define ATLAS_TEXTURE_SLOT_UV_SLOT_SIZE (1.0f / (f32)ATLAS_TEXTURE_SLOT_COUNT)

#define ATLAS_TEXTURE_SLOT_UV_BL(x, y)                                                         \
    { (f32)(x) / (f32)ATLAS_TEXTURE_SLOT_COUNT, (f32)(y) / (f32)ATLAS_TEXTURE_SLOT_COUNT }

#define ATLAS_TEXTURE_SLOT_UV_TL(x, y)                                                         \
    {                                                                                          \
        (f32)(x) / (f32)ATLAS_TEXTURE_SLOT_COUNT,                                              \
            (f32)(y) / (f32)ATLAS_TEXTURE_SLOT_COUNT + ATLAS_TEXTURE_SLOT_UV_SLOT_SIZE         \
    }

#define ATLAS_TEXTURE_SLOT_UV_BR(x, y)                                                         \
    {                                                                                          \
        (f32)(x) / (f32)ATLAS_TEXTURE_SLOT_COUNT + ATLAS_TEXTURE_SLOT_UV_SLOT_SIZE,            \
            (f32)(y) / (f32)ATLAS_TEXTURE_SLOT_COUNT                                           \
    }

#define ATLAS_TEXTURE_SLOT_UV_TR(x, y)                                                         \
    {                                                                                          \
        (f32)(x) / (f32)ATLAS_TEXTURE_SLOT_COUNT + ATLAS_TEXTURE_SLOT_UV_SLOT_SIZE,            \
            (f32)(y) / (f32)ATLAS_TEXTURE_SLOT_COUNT + ATLAS_TEXTURE_SLOT_UV_SLOT_SIZE         \
    }

typedef struct texture {
    u32 id;
    i32 width;
    i32 height;
    i32 channels;
} texture_t;

texture_t texture_load(const char* path);

void texture_free(texture_t* texture);

void texture_bind(texture_t* texture, u32 slot);
