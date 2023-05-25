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
#include "globals.h"
#include "physics.h"
#include "player.h"
#include "types.h"
#include "log.h"
#include "shader.h"
#include "mesh.h"
#include "assets.h"
#include "world.h"

// Disable clang-format for this block
// clang-format off

vertex_t plain_axes_vertices[] = {
    { { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
    { { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
};

u32 plain_axes_indices[] = {
    0, 1, 2, 3, 4, 5,
};

vertex_t cube_skeleton_vertices[] = {
    // Back
    { { -0.505f, -0.505f, -0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.505f, -0.505f, -0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.505f,  0.505f, -0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.505f,  0.505f, -0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    // Front
    { { -0.505f, -0.505f, 0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.505f, -0.505f, 0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.505f,  0.505f, 0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.505f,  0.505f, 0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
};

u32 cube_skeleton_indices[] = {
    0, 1, 1, 3, 3, 2, 2, 0, // Back
    4, 5, 5, 7, 7, 6, 6, 4, // Front
    0, 4, 1, 5, 3, 7, 2, 6, // Sides
};

vertex_t cube_vertices[] = {
    // Back
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BL(0, 0) },
    { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BR(0, 0) },
    { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_TL(0, 0) },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_TR(0, 0) },
    // Front
    { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_BL(0, 0) },
    { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_BR(0, 0) },
    { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TL(0, 0) },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TR(0, 0) },
    // Left
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BL(0, 0) },
    { { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_BR(0, 0) },
    { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_TL(0, 0) },
    { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TR(0, 0) },
    // Right
    { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BL(0, 0) },
    { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_BR(0, 0) },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_TL(0, 0) },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TR(0, 0) },
    // Bottom
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BL(0, 0) },
    { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BR(0, 0) },
    { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TL(0, 0) },
    { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TR(0, 0) },
    // Top
    { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BL(0, 0) },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BR(0, 0) },
    { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TL(0, 0) },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TR(0, 0) },
};

// for GL_TRIANGLES
u32 cube_indices[] = {
    0, 1, 2, 2, 1, 3,
    // front
    4, 6, 5, 5, 6, 7,
    // left
    8, 10, 9, 9, 10, 11,
    // right
    12, 13, 14, 14, 13, 15,
    // bottom
    16, 18, 17, 17, 18, 19,
    // top
    20, 21, 22, 22, 21, 23,
};

vertex_t magic_plane_vertices[] = {
    { { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
    { { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
    { { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
};

u32 magic_plane_indices[] = {
    0, 1, 2, 2, 1, 3,
};

// clang-format on

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

    mesh_t plain_axes = {
        .draw_mode = GL_LINES,
        .vertex_count = 6,
        .index_count = 6,
        .vertices = plain_axes_vertices,
        .indices = plain_axes_indices,
    };

    mesh_init(&plain_axes);

    mesh_t cube_skeleton = {
        .draw_mode = GL_LINES,
        .vertex_count = 8,
        .index_count = 24,
        .vertices = cube_skeleton_vertices,
        .indices = cube_skeleton_indices,
    };

    mesh_init(&cube_skeleton);

    mesh_t magic_plane = {
        .draw_mode = GL_TRIANGLES,
        .vertex_count = 4,
        .index_count = 6,
        .vertices = magic_plane_vertices,
        .indices = magic_plane_indices,
    };

    mesh_init(&magic_plane);

    LOG_INFO("Meshes initialized\n");

    texture_t atlas = texture_load("assets/textures/atlas.png");
    if (!atlas.id) {
        LOG_ERROR("Failed to load texture\n");
        return -1;
    }

    u8 magic_pixel_data[4] = { 255, 255, 255, 255 };
    g_magic_pixel = texture_load_from_memory(magic_pixel_data, 1, 1, 4);
    if (!g_magic_pixel.id) {
        LOG_ERROR("Failed to create magic pixel\n");
        return -1;
    }

    texture_t cursor = texture_load("assets/textures/cursor.png");
    if (!cursor.id) {
        LOG_ERROR("Failed to load cursor texture\n");
        return -1;
    }

    LOG_INFO("Texture loaded\n");

    mesh_instance_t plain_axes_instance = mesh_instance_new(&plain_axes);

    mesh_instance_t cube_skeleton_instance = mesh_instance_new(&cube_skeleton);

    mesh_instance_t cursor_instance = mesh_instance_new(&magic_plane);
    glm_mat4_identity(cursor_instance.transform);
    glm_translate(
        cursor_instance.transform,
        (vec3){ (float)g_window_size[0] / 2.0f - 16.0f,
                (float)g_window_size[1] / 2.0f - 16.0f,
                0.0f }
    );
    glm_scale(cursor_instance.transform, (vec3){ 32.0f, 32.0f, 1.0f });

    camera_t camera =
        camera_new((vec3){ 0.0f, 0.0f, 6.0f }, (vec3){ 0.0f, 0.0f, 0.0f }, GLM_MAT4_IDENTITY);

    glm_perspective(
        glm_rad(90.0f),
        (float)g_window_size[0] / (float)g_window_size[1],
        0.1f,
        100.0f,
        camera.projection
    );

    camera_look_at(&camera, (vec3){ 0.0f, 0.0f, 0.0f }, (vec3){ 0.0f, 1.0f, 0.0f });

    LOG_INFO("Camera initialized\n");

    g_player = player_new((vec3){ 0.0f, 0.0f, 0.0f }, (vec3){ 0.0f, 0.0f, 0.0f }, camera);

    LOG_INFO("Player initialized\n");

    shader_t shader = shader_from_assets("shaders/vertex.glsl", "shaders/fragment.glsl");

    LOG_INFO("Shader initialized\n");

    world_t* world = world_new();
    for (i32 x = 0; x < 8; x++) {
        for (i32 y = 0; y < 2; y++) {
            for (i32 z = 0; z < 8; z++) {
                world_get_or_load_chunk(world, (ivec3){ x, y, z });
            }
        }
    }

    LOG_INFO("World initialized\n");

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

    // Set up depth testing
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

    ivec3 old_selected_block = { 0, 0, 0 };
    ivec3 selected_block = { 0, 0, 0 };
    bool block_selected = false;

    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        shader_use(&shader);
        player_set_uniforms(&g_player, &shader);

        if (g_debug_tools.no_textures) {
            texture_bind(&g_magic_pixel, 0);
        } else {
            texture_bind(&atlas, 0);
        }

        mesh_instance_draw(&plain_axes_instance);

        if (block_selected) {
            mesh_instance_draw(&cube_skeleton_instance);
        }

        world_draw(world);

        // UI
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        shader_set_mat4(&shader, "u_view", ui_view);
        shader_set_mat4(&shader, "u_projection", ui_projection);

        texture_bind(&cursor, 0);

        mesh_instance_draw(&cursor_instance);

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

        player_update(&g_player);

        old_selected_block[0] = selected_block[0];
        old_selected_block[1] = selected_block[1];
        old_selected_block[2] = selected_block[2];

        block_selected = camera_pointed_block(&g_player.camera, world, 8.0f, &selected_block);

        if (block_selected && (old_selected_block[0] != selected_block[0] ||
                               old_selected_block[1] != selected_block[1] ||
                               old_selected_block[2] != selected_block[2])) {
            glm_mat4_identity(cube_skeleton_instance.transform);
            glm_translate(
                cube_skeleton_instance.transform,
                (vec3){ (float)selected_block[0] + 0.5f,
                        (float)selected_block[1] + 0.5f,
                        (float)selected_block[2] + 0.5f }
            );

            LOG_INFO(
                "Selected block: %d %d %d\n",
                selected_block[0],
                selected_block[1],
                selected_block[2]
            );
        }

        if (g_mouse.buttons_down[0]) {
            world_try_set_block_at(world, selected_block, BLOCK_AIR);
        }

        if (block_selected && g_mouse.buttons[1]) {
            vec3 normal;

            LOG_INFO("Normal: %f %f %f\n", normal[0], normal[1], normal[2]);

            ray_t ray;
            ray_from_camera(&ray, &g_player.camera);

            ray_intersect_block(ray, world, 8.0f, BLOCK_FLAG_SOLID, &selected_block, &normal);

            ivec3 block_to_set = { selected_block[0] + (i32)normal[0],
                                   selected_block[1] + (i32)normal[1],
                                   selected_block[2] + (i32)normal[2] };

            LOG_INFO(
                "Block to set: %d %d %d\n",
                block_to_set[0],
                block_to_set[1],
                block_to_set[2]
            );

            world_set_block_at(world, block_to_set, 4);
        }

        for (u32 i = 0; i < 3; i++) {
            g_mouse.buttons[i] = false;
        }
    }

    LOG_INFO("Exiting main loop\n");
    LOG_INFO("Average delta time: %f\n", rolling_avg_delta_time);
    LOG_INFO("Average FPS: %f\n", 1.0f / rolling_avg_delta_time);

    mesh_free(&plain_axes);
    mesh_free(&cube_skeleton);
    mesh_free(&magic_plane);
    shader_free(&shader);

    usize total_chunks = 0;
    usize total_blocks = 0;
    usize total_vertices = 0;
    usize total_tris = 0;

    for (u32 i = 0; i < MAX_LOADED_CHUNKS; i++) {
        chunk_t* chunk = &world->chunks[i];

        if (chunk->mesh.vao != 0) {
            total_chunks++;

            total_vertices += chunk->mesh.vertex_count;
            total_tris += (usize)(chunk->mesh.index_count / 3);

            for (i32 j = 0; j < CHUNK_SIZE; j++) {
                for (i32 k = 0; k < CHUNK_SIZE; k++) {
                    for (i32 l = 0; l < CHUNK_SIZE; l++) {
                        if (chunk_get_block(chunk, (ivec3){ j, k, l }).id != BLOCK_AIR) {
                            total_blocks++;
                        }
                    }
                }
            }
        }
    }

    world_free(world);

    LOG_INFO("Total chunks: %zu\n", total_chunks);
    LOG_INFO("Total blocks: %zu\n", total_blocks);
    LOG_INFO("Total vertices: %zu\n", total_vertices);
    LOG_INFO("Total triangles: %zu\n", total_tris);

    glfwTerminate();
    return 0;
}
