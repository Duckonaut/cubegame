#include "shader.h"

#include "log.h"
#include "types.h"
#include "assets.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <cglm/types.h>

shader_t g_shader_current;

shader_t shader_new(const char* vertex_shader_source, const char* fragment_shader_source) {
    shader_t shader = { 0 };

    u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    GLint success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        LOG_ERROR("Failed to compile vertex shader\n");

        i32 log_length;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
        char* log = malloc((usize)log_length);
        glGetShaderInfoLog(vertex_shader, log_length, NULL, log);
        LOG_ERROR("Shader compilation log: %s\n", log);
        free(log);

        return shader;
    }

    u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        LOG_ERROR("Failed to compile fragment shader\n");

        i32 log_length;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
        char* log = malloc((usize)log_length);
        glGetShaderInfoLog(fragment_shader, log_length, NULL, log);
        LOG_ERROR("Shader compilation log: %s\n", log);
        free(log);

        return shader;
    }

    shader.program = glCreateProgram();
    glAttachShader(shader.program, vertex_shader);
    glAttachShader(shader.program, fragment_shader);

    glLinkProgram(shader.program);

    glGetProgramiv(shader.program, GL_LINK_STATUS, &success);
    if (!success) {
        LOG_ERROR("Failed to link shader program\n");

        i32 log_length;
        glGetProgramiv(shader.program, GL_INFO_LOG_LENGTH, &log_length);
        char* log = malloc((usize)log_length);
        glGetProgramInfoLog(shader.program, log_length, NULL, log);
        LOG_ERROR("Shader link log: %s\n", log);
        free(log);

        return shader;
    }

    glDetachShader(shader.program, vertex_shader);
    glDetachShader(shader.program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader;
}
shader_t shader_from_assets(const char* vertex_shader_path, const char* fragment_shader_path) {
    char* vertex_shader_source = (char*)read_asset(vertex_shader_path, NULL);
    char* fragment_shader_source = (char*)read_asset(fragment_shader_path, NULL);

    if (!vertex_shader_source || !fragment_shader_source) {
        LOG_ERROR("Failed to read shader source files\n");
        return (shader_t){ 0 };
    }

    shader_t shader = shader_new(vertex_shader_source, fragment_shader_source);

    free(vertex_shader_source);
    free(fragment_shader_source);

    return shader;
}

void shader_free(shader_t* shader) {
    glDeleteProgram(shader->program);
}

void shader_use(shader_t* shader) {
    glUseProgram(shader->program);

    g_shader_current = *shader;
}

void shader_set_int(shader_t* shader, const char* name, i32 value) {
    glUniform1i(glGetUniformLocation(shader->program, name), value);
}
void shader_set_uint(shader_t* shader, const char* name, u32 value) {
    glUniform1ui(glGetUniformLocation(shader->program, name), value);
}
void shader_set_float(shader_t* shader, const char* name, f32 value) {
    glUniform1f(glGetUniformLocation(shader->program, name), value);
}
void shader_set_vec2(shader_t* shader, const char* name, vec2 value) {
    glUniform2f(glGetUniformLocation(shader->program, name), value[0], value[1]);
}
void shader_set_vec3(shader_t* shader, const char* name, vec3 value) {
    glUniform3f(glGetUniformLocation(shader->program, name), value[0], value[1], value[2]);
}
void shader_set_vec4(shader_t* shader, const char* name, vec4 value) {
    glUniform4f(
        glGetUniformLocation(shader->program, name),
        value[0],
        value[1],
        value[2],
        value[3]
    );
}
void shader_set_mat4(shader_t* shader, const char* name, mat4 value) {
    glUniformMatrix4fv(glGetUniformLocation(shader->program, name), 1, GL_FALSE, (f32*)value);
}

void shader_set_uint_array(shader_t *shader, const char *name, u32 *values, i32 count) {
    glUniform1uiv(glGetUniformLocation(shader->program, name), count, values);
}
