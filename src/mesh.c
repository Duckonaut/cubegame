#include "mesh.h"
#include "glm_extra.h"
#include "shader.h"
#include "globals.h"
#include <cglm/mat4.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(vertex_t),
        (void*)offsetof(vertex_t, uv)
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
        .active = true,
    };

    instance.color[0] = 1.0f;
    instance.color[1] = 1.0f;
    instance.color[2] = 1.0f;
    instance.color[3] = 1.0f;

    return instance;
}

void mesh_instance_draw(mesh_instance_t* instance) {
    if (!instance->active) {
        return;
    }
    shader_set_mat4(&g_shader_current, "u_model", instance->transform);
    shader_set_vec4(&g_shader_current, "u_color", instance->color);

    mesh_draw(instance->mesh);
}

void mesh_instance_batch_draw(mesh_instance_t *instances, usize count) {
    if (count == 0) {
        return;
    }
    glBindVertexArray(instances[0].mesh->vao);

    for (usize i = 0; i < count; i++) {
        if (!instances[i].active) {
            continue;
        }
        shader_set_mat4(&g_shader_current, "u_model", instances[i].transform);
        shader_set_vec4(&g_shader_current, "u_color", instances[i].color);
        glDrawElements(
            instances[i].mesh->draw_mode,
            instances[i].mesh->index_count,
            GL_UNSIGNED_INT,
            instances[i].mesh->indices
        );
    }

    glBindVertexArray(0);
}
