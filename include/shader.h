#pragma once
#include "types.h"
#include <cglm/types.h>

typedef struct shader {
    u32 program;
} shader_t;

extern shader_t g_shader_current;

shader_t shader_new(const char* vertex_shader_source, const char* fragment_shader_source);
shader_t shader_from_assets(const char* vertex_shader_path, const char* fragment_shader_path);
void shader_free(shader_t* shader);

void shader_use(shader_t* shader);

void shader_set_int(shader_t* shader, const char* name, int value);
void shader_set_uint(shader_t* shader, const char* name, u32 value);
void shader_set_float(shader_t* shader, const char* name, float value);
void shader_set_vec2(shader_t* shader, const char* name, vec2 value);
void shader_set_vec3(shader_t* shader, const char* name, vec3 value);
void shader_set_vec4(shader_t* shader, const char* name, vec4 value);
void shader_set_mat4(shader_t* shader, const char* name, mat4 value);
