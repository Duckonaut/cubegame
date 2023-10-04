#pragma once

#include "types.h"
#include <cglm/types.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef struct vertex {
    vec3 position;
    vec3 normal;
    vec2 uv;
    u32 shadow_index;
} vertex_t;

typedef struct mesh {
    GLenum draw_mode;
    GLuint vao;
    GLuint vbo;
    u32 vertex_count;
    i32 index_count;
    vertex_t* vertices;
    u32* indices;
} mesh_t;

void mesh_init(mesh_t* mesh);

void mesh_free(mesh_t* mesh);

void mesh_draw(mesh_t* mesh);

typedef struct mesh_instance {
    mesh_t* mesh;
    mat4 transform;
    vec4 color;
    bool active;
} mesh_instance_t;

mesh_instance_t mesh_instance_new(mesh_t* mesh);

void mesh_instance_draw(mesh_instance_t* instance);

// Requires all mesh instances to have the same mesh
void mesh_instance_batch_draw(mesh_instance_t* instances, usize count);
