#include "globals.h"

#include "types.h"
#include <cglm/cglm.h>

int g_window_size[2] = { 0 };

mouse_t g_mouse = { 0 };

keyboard_t g_keyboard = { 0 };

GLFWwindow* g_window = NULL;

gametime_t g_gametime = { 0 };
