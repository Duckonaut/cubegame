#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <cglm/types.h>
#include <cglm/vec3.h>

#include <time.h>

#include "camera.h"
#include "globals.h"
#include "types.h"
#include "log.h"
#include "shader.h"

typedef struct vertex {
    vec3 position;
    vec3 color;
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

typedef struct mesh_instance {
    mesh_t* mesh;
    mat4 transform;
} mesh_instance_t;

vertex_t vertices[] = {
    { { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    { { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
};

u32 indices[] = {
    0, 1, 2, 3, 4, 5,
};

void mesh_init(mesh_t* mesh) {
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        mesh->vertex_count * sizeof(vertex_t),
        mesh->vertices,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(vertex_t),
        (void*)offsetof(vertex_t, color)
    );

    glBindVertexArray(0);
}

void mesh_draw(mesh_t* mesh) {
    glBindVertexArray(mesh->vao);
    glDrawElements(mesh->draw_mode, mesh->index_count, GL_UNSIGNED_INT, mesh->indices);
    glBindVertexArray(0);
}

void mesh_free(mesh_t* mesh) {
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
}

mesh_instance_t mesh_instance_new(mesh_t* mesh) {
    mesh_instance_t instance = {
        .mesh = mesh,
        .transform = GLM_MAT4_IDENTITY_INIT,
    };

    return instance;
}

void mesh_instance_draw(mesh_instance_t* instance) {
    shader_set_mat4(&g_shader_current, "u_model", instance->transform);
    mesh_draw(instance->mesh);
}

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
    (void)scancode; // unused
    (void)mods;     // unused

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
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
        g_mouse.buttons[button_index] = true;
    } else if (action == GLFW_RELEASE) {
        g_mouse.buttons[button_index] = false;
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
    window = glfwCreateWindow(640, 480, "dev: cubegame", NULL, NULL);
    g_window_size[0] = 640;
    g_window_size[1] = 480;

    if (!window) {
        glfwTerminate();
        return -1;
    }

    LOG_INFO("Window created\n");

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
        .vertices = vertices,
        .indices = indices,
    };

    mesh_init(&plain_axes);

    LOG_INFO("Mesh initialized\n");

    mesh_instance_t plain_axes_instance = mesh_instance_new(&plain_axes);

    camera_t camera =
        camera_new((vec3){ 0.0f, 0.0f, 1.0f }, (vec3){ 0.0f, 0.0f, 0.0f }, GLM_MAT4_IDENTITY);

    glm_perspective(glm_rad(90.0f), 640.0f / 480.0f, 0.1f, 100.0f, camera.projection);

    camera_look_at(&camera, (vec3){ 0.0f, 0.0f, 0.0f }, (vec3){ 0.0f, 1.0f, 0.0f });

    shader_t shader = shader_from_assets("shaders/vertex.glsl", "shaders/fragment.glsl");

    LOG_INFO("Entering main loop\n");

    // clock_t last_time = clock();
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        // clock_t current_time = clock();
        // float delta_time = (float)(current_time - last_time) / CLOCKS_PER_SEC;
        // last_time = current_time;
        //
        // LOG_INFO("Delta time: %f\n", delta_time);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        shader_use(&shader);
        shader_set_mat4(&shader, "u_view", camera.view);
        shader_set_mat4(&shader, "u_projection", camera.projection);

        mesh_instance_draw(&plain_axes_instance);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        g_mouse.last_position[0] = g_mouse.position[0];
        g_mouse.last_position[1] = g_mouse.position[1];
        /* Poll for and process events */
        glfwPollEvents();

        g_mouse.delta[0] = g_mouse.position[0] - g_mouse.last_position[0];
        g_mouse.delta[1] = g_mouse.position[1] - g_mouse.last_position[1];

        camera_update(&camera);
    }

    mesh_free(&plain_axes);
    shader_free(&shader);

    glfwTerminate();
    return 0;
}
