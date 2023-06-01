#include "globals.h"

#include "game.h"
#include "player.h"
#include "types.h"
#include "world.h"
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
        if (key == GLFW_KEY_F1) {
            g_player.movement_mode += 1;
            if (g_player.movement_mode >= PLAYER_MOVEMENT_MODE_COUNT) {
                g_player.movement_mode = 0;
            }
        } else if (key == GLFW_KEY_F2) {
            // reserved
        } else if (key == GLFW_KEY_F3) {
            g_debug_tools.show_wireframe = !g_debug_tools.show_wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, g_debug_tools.show_wireframe ? GL_LINE : GL_FILL);
            g_debug_tools.thin_lines = !g_debug_tools.thin_lines;
            glLineWidth(g_debug_tools.thin_lines ? 1.0f : 4.0f);
        } else if (key == GLFW_KEY_F4) {
            g_debug_tools.no_textures = !g_debug_tools.no_textures;
        } else if (key == GLFW_KEY_F5) {
            g_debug_tools.no_lighting = !g_debug_tools.no_lighting;
        } else if (key == GLFW_KEY_F6) {
            g_debug_tools.no_cull = !g_debug_tools.no_cull;
            if (g_debug_tools.no_cull) {
                glDisable(GL_CULL_FACE);
            } else {
                glEnable(GL_CULL_FACE);
            }
        } else if (key == GLFW_KEY_F7) {
            g_debug_tools.no_chunk_load = !g_debug_tools.no_chunk_load;
        } else if (key == GLFW_KEY_F8) {
            g_debug_tools.force_day = !g_debug_tools.force_day;
        } else if (key == GLFW_KEY_R) {
            world_free(g_game.world);
            g_game.world = world_new();
            g_player.position[0] = 0.0f;
            g_player.position[1] = 0.0f;
            g_player.position[2] = 0.0f;
        }
    }
}
