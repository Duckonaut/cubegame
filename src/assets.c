#include "assets.h"

#include "types.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

texture_t g_magic_pixel;

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

void dump_buffer(const u8* buffer, usize buffer_size, usize width) {
    printf("buffer_size: %ld\n", buffer_size);
    printf("buffer: [\n");
    for (usize i = 0; i < buffer_size; i++) {
        printf("\t0x%02x,", buffer[i]);
        if (i % width == width - 1) {
            printf("\n");
        }
    }
    printf("\n]\n");
}

texture_t texture_load(const char* path) {
    texture_t texture = { 0 };

    u8* image_data = stbi_load(path, &texture.width, &texture.height, &texture.channels, 0);

    if (!image_data) {
        LOG_ERROR("Failed to load texture: %s\n", path);
        return texture;
    }

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    LOG_DEBUG(
        "Loaded texture %s (%dx%d:%d)\n",
        path,
        texture.width,
        texture.height,
        texture.channels
    );

    if (texture.channels == 3) {
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            texture.width,
            texture.height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            image_data
        );
    } else if (texture.channels == 4) {
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            texture.width,
            texture.height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            image_data
        );
    } else {
        LOG_ERROR("Unsupported texture format: %s\n", path);
        stbi_image_free(image_data);
        texture.id = 0;
        return texture;
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image_data);

    return texture;
}

texture_t texture_load_from_memory(u8* data, i32 width, i32 height, i32 channels) {
    texture_t texture = { 0 };

    texture.width = width;
    texture.height = height;
    texture.channels = channels;

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (texture.channels == 3) {
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            texture.width,
            texture.height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            data
        );
    } else if (texture.channels == 4) {
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            texture.width,
            texture.height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data
        );
    } else {
        LOG_ERROR("Unsupported texture format\n");
        texture.id = 0;
        return texture;
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

void texture_free(texture_t* texture) {
    glDeleteTextures(1, &texture->id);
}

void texture_bind(texture_t* texture, u32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}
