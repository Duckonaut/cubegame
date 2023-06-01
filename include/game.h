#pragma once

#include "assets.h"
#include "mesh.h"
#include "shader.h"
#include "types.h"
#include "ui.h"
#include "world.h"

typedef void (*window_size_callback)(int width, int height);

typedef struct content {
    texture_t cursor;
    texture_t atlas;
    texture_t font;
    texture_t ui_atlas;
    texture_t sun;

    shader_t sprite_shader;
    shader_t world_shader;
    shader_t unlit_shader;
    shader_t gizmo_shader;
    shader_t ui_shader;

    mesh_t quad;
    mesh_t plain_axes;
    mesh_t cube_skeleton;
} content_t;

typedef struct instances {
    mesh_instance_t plain_axes_instance;
    mesh_instance_t cube_skeleton_instance;
    mesh_instance_t sun_instance;
} instances_t;

typedef struct game_state {
    content_t content;
    instances_t instances;
    ui_t ui;
    world_t* world; // big, stored on heap
    f32 time;
    vec3 sky_color;
} game_state_t;

extern game_state_t g_game;

int game_load_content(void);

int game_init(void);

void game_update(f32 delta_time);

void game_draw(void);

void game_draw_debug(void);

void game_draw_ui(void);

void game_free(void);
