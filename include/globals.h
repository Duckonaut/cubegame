#pragma once

#include "types.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/types.h>

extern int g_window_size[2];

typedef struct mouse {
    vec2 position;
    vec2 last_position;
    vec2 delta;
    bool captured;
    bool buttons[3];
    bool buttons_down[3];
    bool buttons_up[3];
} mouse_t;

extern mouse_t g_mouse;

typedef struct keyboard {
    int mods;
    bool keys[GLFW_KEY_LAST];
} keyboard_t;

extern keyboard_t g_keyboard;

extern GLFWwindow* g_window;

typedef struct gametime {
    float delta_time;
    float total_time;
} gametime_t;

extern gametime_t g_gametime;

typedef struct debug_tools {
    bool show_wireframe;
    bool no_textures;
    bool thin_lines;
    bool no_cull;
    bool no_chunk_load;
    bool no_lighting;
    bool force_day;
} debug_tools_t;

extern debug_tools_t g_debug_tools;

void debug_tools_key_callback(int key, int action, int mods);
