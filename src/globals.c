#include "globals.h"

#include "types.h"
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

int g_window_size[2] = { 0 };

mouse_t g_mouse = { 0 };

keyboard_t g_keyboard = { 0 };

GLFWwindow* g_window = NULL;

gametime_t g_gametime = { 0 };

debug_tools_t g_debug_tools = { 0 };

void debug_tools_key_callback(int key, int action, int mods) {
    (void)mods; // unused

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_F3) {
            g_debug_tools.show_wireframe = !g_debug_tools.show_wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, g_debug_tools.show_wireframe ? GL_LINE : GL_FILL);
        } else if (key == GLFW_KEY_F4) {
            g_debug_tools.no_textures = !g_debug_tools.no_textures;
        } else if (key == GLFW_KEY_F5) {
            g_debug_tools.thin_lines = !g_debug_tools.thin_lines;
            glLineWidth(g_debug_tools.thin_lines ? 1.0f : 4.0f);
        } else if (key == GLFW_KEY_F6) {
            g_debug_tools.no_cull = !g_debug_tools.no_cull;
            if (g_debug_tools.no_cull) {
                glDisable(GL_CULL_FACE);
            } else {
                glEnable(GL_CULL_FACE);
            }
        }
    }
}
