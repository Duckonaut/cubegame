#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <cglm/vec3.h>

#include <stdlib.h>
#include <time.h>

#include "camera.h"
#include "game.h"
#include "globals.h"
#include "physics.h"
#include "player.h"
#include "types.h"
#include "log.h"
#include "shader.h"
#include "mesh.h"
#include "assets.h"
#include "world.h"
#include "asset_data.h"

static void glfw_error_callback(int error, const char* description) {
    LOG_ERROR("GLFW Error %d: %s", error, description);
}

static void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
    (void)window; // unused

    g_window_size[0] = width;
    g_window_size[1] = height;

    LOG_INFO("Window resized to %dx%d\n", width, height);
}

static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window; // unused

    glViewport(0, 0, width, height);
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)window;   // unused
    (void)scancode; // unused

    g_keyboard.keys[key] = action != GLFW_RELEASE;
    g_keyboard.mods = mods;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        g_mouse.captured = false;
    }

    debug_tools_key_callback(key, action, mods);
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window; // unused
    (void)mods;   // unused

    usize button_index = 0;
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            button_index = 0;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            button_index = 1;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            button_index = 2;
            break;
        default:
            return;
    }

    if (action == GLFW_PRESS) {
        if (g_mouse.buttons_up[button_index]) {
            g_mouse.buttons[button_index] = true;
        }

        g_mouse.buttons_down[button_index] = true;
        g_mouse.buttons_up[button_index] = false;
    } else if (action == GLFW_RELEASE) {
        g_mouse.buttons_down[button_index] = false;
        g_mouse.buttons_up[button_index] = true;
    }

    if (button_index == 0 && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        g_mouse.captured = true;
    }
}

static void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window; // unused

    g_mouse.position[0] = (float)xpos;
    g_mouse.position[1] = (float)ypos;
}

int main(void) {
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(960, 640, "dev: cubegame", NULL, NULL);
    g_window_size[0] = 960;
    g_window_size[1] = 640;

    if (!window) {
        glfwTerminate();
        return -1;
    }

    LOG_INFO("statically included asset data size: %zu\n", sizeof(a_asset_data));

    LOG_INFO("Window created\n");

    g_window = window;

    glfwSetErrorCallback(glfw_error_callback);
    glfwSetWindowSizeCallback(window, glfw_window_size_callback);
    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetCursorPosCallback(window, glfw_cursor_position_callback);

    LOG_INFO("GLFW callbacks set\n");

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    LOG_INFO("Context made current\n");

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        LOG_ERROR("Failed to initialize GLEW: %s\n", glewGetErrorString(err));
        return -1;
    }

    LOG_INFO("OpenGL Version: %s\n", glGetString(GL_VERSION));

    if (game_load_content() != 0) {
        LOG_ERROR("Failed to load game content\n");
        return -1;
    } else {
        LOG_INFO("Game content loaded\n");
    }

    if (game_init() != 0) {
        LOG_ERROR("Failed to initialize game\n");
        return -1;
    }

    LOG_INFO("Game initialized\n");

    mat4 ui_projection;
    glm_ortho(
        0.0f,
        (float)g_window_size[0],
        0.0f,
        (float)g_window_size[1],
        -1.0f,
        1.0f,
        ui_projection
    );

    mat4 ui_view = GLM_MAT4_IDENTITY_INIT;

    LOG_INFO("UI projection and view matrices initialized\n");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glLineWidth(4.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    LOG_INFO("Entering main loop\n");

    float rolling_avg_delta_time = 0.0f;

    double last_time = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.4f, 0.8f, 1.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);

        game_draw();

        game_draw_debug();

        // UI
        glDisable(GL_DEPTH_TEST);

        shader_use(&g_game.content.ui_shader);

        shader_set_mat4(&g_game.content.ui_shader, "u_view", ui_view);
        shader_set_mat4(&g_game.content.ui_shader, "u_projection", ui_projection);

        game_draw_ui();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        g_mouse.last_position[0] = g_mouse.position[0];
        g_mouse.last_position[1] = g_mouse.position[1];

        /* Poll for and process events */
        glfwPollEvents();

        g_mouse.delta[0] = g_mouse.position[0] - g_mouse.last_position[0];
        g_mouse.delta[1] = g_mouse.position[1] - g_mouse.last_position[1];

        double current_time = glfwGetTime();
        g_gametime.delta_time = (float)(current_time - last_time);
        last_time = current_time;

        rolling_avg_delta_time = rolling_avg_delta_time * 0.9f + g_gametime.delta_time * 0.1f;

        game_update();

        for (u32 i = 0; i < 3; i++) {
            g_mouse.buttons[i] = false;
        }
    }

    LOG_INFO("Exiting main loop\n");
    LOG_INFO("Average delta time: %f\n", rolling_avg_delta_time);
    LOG_INFO("Average FPS: %f\n", 1.0f / rolling_avg_delta_time);

    usize total_chunks = 0;
    usize total_blocks = 0;
    usize total_vertices = 0;
    usize total_tris = 0;

    for (u32 i = 0; i < MAX_LOADED_CHUNKS; i++) {
        chunk_t* chunk = &g_game.world->chunks[i];

        if (chunk->mesh.vao != 0) {
            total_chunks++;

            total_vertices += chunk->mesh.vertex_count;
            total_tris += (usize)(chunk->mesh.index_count / 3);

            for (i32 j = 0; j < CHUNK_SIZE; j++) {
                for (i32 k = 0; k < CHUNK_SIZE; k++) {
                    for (i32 l = 0; l < CHUNK_SIZE; l++) {
                        if (chunk_get_block(chunk, (ivec3){ j, k, l })->id != BLOCK_AIR) {
                            total_blocks++;
                        }
                    }
                }
            }
        }
    }

    game_free();

    LOG_INFO("Total chunks: %zu\n", total_chunks);
    LOG_INFO("Total blocks: %zu\n", total_blocks);
    LOG_INFO("Total vertices: %zu\n", total_vertices);
    LOG_INFO("Total triangles: %zu\n", total_tris);

    glfwTerminate();
    return 0;
}
