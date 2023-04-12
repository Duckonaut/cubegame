#pragma once

#include "types.h"

#define ASSET_PATH "assets/"

u8* read_file(const char* path, usize* size);
u8* read_asset(const char* path, usize* size);

void free_file(u8* file);
