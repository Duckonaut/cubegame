#include "saves.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <cglm/vec3.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#include <generator/output/cimgui_impl.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
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
#include "ui.h"
#include "world.h"
#include "asset_data.h"

#define FRAMETIME_SAMPLES 2000

static void glfw_error_callback(int error, const char* description) {
    LOG_ERROR("GLFW Error %d: %s", error, description);
}

static void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
    (void)window; // unused

    g_window_size[0] = width;
    g_window_size[1] = height;

    ui_update(&g_game.ui);

    glm_ortho(
        0.0f,
        (float)g_window_size[0],
        0.0f,
        (float)g_window_size[1],
        -1.0f,
        1.0f,
        g_game.ui.projection
    );

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
        // enable imgui mouse capture
        ImGuiIO* io = igGetIO();
        io->MouseDrawCursor = true;
        io->ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        io->ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;

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
        // check if the mouse is captured by imgui
        ImGuiIO* io = igGetIO();
        if (io->WantCaptureMouse) {
            return;
        }

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // disable imgui mouse capture
        io->ConfigFlags |= ImGuiConfigFlags_NoMouse;

        g_mouse.captured = true;
    }
}

static void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window; // unused

    g_mouse.position[0] = (float)xpos;
    g_mouse.position[1] = (float)ypos;
}

// float comparer for qsort for ordering from highest to lowest
static int float_compare(const void* a, const void* b) {
    float fa = *(const float*)a;
    float fb = *(const float*)b;

    return (fa < fb) - (fa > fb);
}

typedef struct args {
    bool vsync;      // -v, --vsync
    char* save_path; // -s, --save
} args_t;

static args_t parse_args(int argc, char** argv) {
    args_t args = {
        .vsync = false,
        .save_path = NULL,
    };

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--vsync") == 0) {
            args.vsync = true;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--save") == 0) {
            if (i + 1 < argc) {
                args.save_path = argv[i + 1];
            } else {
                LOG_ERROR("No save path specified\n");
                exit(1);
            }
        }
    }

    return args;
}

int main(int argc, char** argv) {
    args_t args = parse_args(argc, argv);

    if (args.save_path == NULL) {
        LOG_INFO("No save path specified, creating new save\n");
        g_save = save_new();
    } else {
        g_save = save_load(args.save_path);

        if (g_save == NULL) {
            LOG_ERROR("Failed to load save %s\n", args.save_path);
            // if we would overwrite the save, we should probably not do that
            if (strcmp(args.save_path, "save.cgsv") == 0) {
                LOG_ERROR("Not overwriting default save\n");
                exit(1);
            } else {
                LOG_INFO("Creating new save\n");
                g_save = save_new();
            }
        } else {
            for (size_t i = 0; i < g_save->world.chunk_count; i++) {
                LOG_INFO(
                    "Loaded chunk %d %d %d\n",
                    g_save->world.chunks[i].x,
                    g_save->world.chunks[i].y,
                    g_save->world.chunks[i].z
                );
            }
        }
    }

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

        // if wayland, force vsync
#ifdef __linux__
    if (getenv("WAYLAND_DISPLAY") != NULL) {
        args.vsync = true;
    }
#endif

    if (args.vsync) {
        glfwWindowHint(GLFW_DOUBLEBUFFER, true);
    } else {
        glfwWindowHint(GLFW_DOUBLEBUFFER, false);
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(960, 640, "dev: cubegame", NULL, NULL);
    g_window_size[0] = 960;
    g_window_size[1] = 640;

    if (!window) {
        glfwTerminate();
        return -1;
    }

    log_init();

    LOG_INFO("Logging initialized\n");

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

    LOG_INFO("Setting up ImGui\n");

    igCreateContext(NULL);
    ImGuiIO* io = igGetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    igStyleColorsDark(NULL);

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

    glm_ortho(
        0.0f,
        (float)g_window_size[0],
        0.0f,
        (float)g_window_size[1],
        -1.0f,
        1.0f,
        g_game.ui.projection
    );

    glm_mat4_identity(g_game.ui.view);

    LOG_INFO("UI projection and view matrices initialized\n");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glLineWidth(4.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    LOG_INFO("OpenGL state set\n");

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    LOG_INFO("Entering main loop\n");

    float frametimes[FRAMETIME_SAMPLES];
    memset(frametimes, 0, sizeof(float) * FRAMETIME_SAMPLES);
    usize frametime_index = 0;
    usize frametime_count = 0;

    double last_time = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(g_game.sky_color[0], g_game.sky_color[1], g_game.sky_color[2], 1.0f);
        glEnable(GL_DEPTH_TEST);

        game_draw();

        game_draw_debug();

        // UI
        glDisable(GL_DEPTH_TEST);

        shader_use(&g_game.content.ui_shader);

        game_draw_ui();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        igNewFrame();

        {
            if (g_mouse.captured) {
                igSetMouseCursor(ImGuiMouseCursor_None);
            }
            igBegin("Debug", NULL, 0);

            igText(
                "Selected block: %d %d %d\n",
                g_player.selected_block[0],
                g_player.selected_block[1],
                g_player.selected_block[2]
            );
            igText("FPS: %f", 1.0f / g_gametime.delta_time);

            igText("Frametimes:");
            // plot frametimes
            igPlotLines_FloatPtr(
                "",
                frametimes,
                FRAMETIME_SAMPLES,
                0, // just loop around
                NULL,
                0.0f,
                0.1f,
                (ImVec2){ 0, 120 },
                sizeof(float)
            );

            igEnd();
        }

        igRender();
        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

        if (args.vsync) {
            glfwSwapBuffers(window);
        } else {
            glFinish();
        }

        g_mouse.last_position[0] = g_mouse.position[0];
        g_mouse.last_position[1] = g_mouse.position[1];

        /* Poll for and process events */
        glfwPollEvents();

        g_mouse.delta[0] = g_mouse.position[0] - g_mouse.last_position[0];
        g_mouse.delta[1] = g_mouse.position[1] - g_mouse.last_position[1];

        double current_time = glfwGetTime();
        g_gametime.delta_time = (float)(current_time - last_time);
        last_time = current_time;

        frametimes[frametime_index] = g_gametime.delta_time;
        frametime_index = (frametime_index + 1) % FRAMETIME_SAMPLES;
        frametime_count =
            frametime_count < FRAMETIME_SAMPLES ? frametime_count + 1 : frametime_count;

        game_update(g_gametime.delta_time);

        for (u32 i = 0; i < 3; i++) {
            g_mouse.buttons[i] = false;
        }
    }

    LOG_INFO("Exiting main loop\n");
    // calculate average frametime, the 1% low and the 0.1% low
    float average_frametime = 0.0f;
    float average_frametime_without_1_percent_low = 0.0f;
    float one_percent_low = 0.0f;
    float point_one_percent_low = 0.0f;

    for (usize i = 0; i < frametime_count; i++) {
        average_frametime += frametimes[i];

        if (i >= (usize)((float)frametime_count * 0.01f)) {
            average_frametime_without_1_percent_low += frametimes[i];
        }
    }

    average_frametime /= (float)frametime_count;
    average_frametime_without_1_percent_low /=
        (float)frametime_count - (float)frametime_count * 0.01f;

    float* sorted_frametimes = malloc(sizeof(float) * frametime_count);
    memcpy(sorted_frametimes, frametimes, sizeof(float) * frametime_count);

    qsort(sorted_frametimes, frametime_count, sizeof(float), float_compare);

    one_percent_low = sorted_frametimes[(usize)((float)frametime_count * 0.01f)];
    point_one_percent_low = sorted_frametimes[(usize)((float)frametime_count * 0.001f)];

    free(sorted_frametimes);

    LOG_INFO("Average frametime: %f\n", average_frametime);
    LOG_INFO(
        "Average frametime without 1%% low: %f\n",
        average_frametime_without_1_percent_low
    );
    LOG_INFO("1%% low: %f\n", one_percent_low);
    LOG_INFO("0.1%% low: %f\n", point_one_percent_low);

    LOG_INFO("Average FPS: %f\n", 1.0f / average_frametime);
    LOG_INFO(
        "Average FPS without 1%% low: %f\n",
        1.0f / average_frametime_without_1_percent_low
    );
    LOG_INFO("1%% low FPS: %f\n", 1.0f / one_percent_low);
    LOG_INFO("0.1%% low FPS: %f\n", 1.0f / point_one_percent_low);

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

        if (chunk->save_dirty) {
            save_add_chunk(g_save, chunk);
        }
    }

    if (args.save_path != NULL) {
        save_write(g_save, args.save_path);
    } else {
        save_write(g_save, "save.cgsv");
    }

    save_free(g_save);
    g_save = NULL;

    game_free();

    LOG_INFO("Total chunks: %zu\n", total_chunks);
    LOG_INFO("Total blocks: %zu\n", total_blocks);
    LOG_INFO("Total vertices: %zu\n", total_vertices);
    LOG_INFO("Total triangles: %zu\n", total_tris);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(NULL);

    glfwTerminate();

    log_close();
    return 0;
}
