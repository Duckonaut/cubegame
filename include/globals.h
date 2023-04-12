#pragma once

#include "types.h"
#include <cglm/types.h>

extern int g_window_size[2];

typedef struct mouse {
    vec2 position;
    vec2 last_position;
    vec2 delta;
    bool buttons[3];
} mouse_t;

extern mouse_t g_mouse;
