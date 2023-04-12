#include "assets.h"

#include "types.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u8* read_file(const char* path, usize* size) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        LOG_ERROR("Failed to open file: %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    usize file_size = (usize)ftell(file);
    fseek(file, 0, SEEK_SET);

    u8* buffer = malloc(file_size + 1);
    fread(buffer, file_size, 1, file);
    buffer[file_size] = '\0';

    fclose(file);

    if (size) {
        *size = file_size;
    }

    return buffer;
}
u8* read_asset(const char* path, usize* size) {
    char* asset_path = malloc(strlen(path) + strlen(ASSET_PATH) + 1);
    strcpy(asset_path, ASSET_PATH);
    strcat(asset_path, path);

    u8* buffer = read_file(asset_path, size);

    free(asset_path);

    return buffer;
}

void free_file(u8* buffer) {
    free(buffer);
}
