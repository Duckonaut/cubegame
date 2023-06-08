#include "lighting.h"

#include "game.h"
#include "globals.h"
#include "log.h"
#include "player.h"
#include "shader.h"
#include "types.h"
#include "world.h"

#include <GL/glew.h>
#include <cglm/cam.h>
#include <cglm/cglm.h>

void shadow_map_init(shadow_map_t* shadow_map) {
    glGenFramebuffers(1, &shadow_map->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map->fbo);

    glGenTextures(1, &shadow_map->texture);
    glBindTexture(GL_TEXTURE_2D, shadow_map->texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT,
        SHADOW_MAP_SIZE,
        SHADOW_MAP_SIZE,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        shadow_map->texture,
        0
    );

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Shadow map framebuffer incomplete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void shadow_map_bind(shadow_map_t* shadow_map) {
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map->fbo);
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void shadow_map_unbind(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, g_window_size[0], g_window_size[1]);
}

void shadow_map_free(shadow_map_t* shadow_map) {
    glDeleteFramebuffers(1, &shadow_map->fbo);
    glDeleteTextures(1, &shadow_map->texture);
}

void light_sun_init(
    light_sun_t* light_sun,
    vec3 position,
    vec3 direction,
    vec3 color,
    f32 intensity
) {
    glm_vec3_copy(position, light_sun->position);
    glm_vec3_copy(direction, light_sun->direction);
    glm_vec3_copy(color, light_sun->color);
    light_sun->intensity = intensity;

    shadow_map_init(&light_sun->shadow_map);

    mat4 light_projection;
    glm_ortho(-40.0f, 40.0f, -40.0f, 40.0f, 0.1f, 200.0f, light_projection);

    mat4 light_view;
    glm_look(light_sun->position, light_sun->direction, (vec3){ 0.0f, 1.0f, 0.0f }, light_view);

    glm_mat4_mul(light_projection, light_view, light_sun->light_view_projection);
}

void light_sun_free(light_sun_t* light_sun) {
    shadow_map_free(&light_sun->shadow_map);
}

void light_sun_shadow_update(light_sun_t* light_sun) {
    mat4 light_projection;
    glm_ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.1f, 200.0f, light_projection);

    mat4 light_view;
    glm_look(
        light_sun->position,
        light_sun->direction,
        (vec3){ 0.0f, 1.0f, 0.0f },
        light_view
    );

    glm_mat4_mul(light_projection, light_view, light_sun->light_view_projection);
    shadow_map_bind(&light_sun->shadow_map);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    shader_use(&g_game.content.shadow_shader);

    shader_set_mat4(
        &g_game.content.shadow_shader,
        "u_light_view_projection",
        light_sun->light_view_projection
    );

    world_draw(g_game.world);

    glEnable(GL_CULL_FACE);
    shadow_map_unbind();
}

void light_sun_shadow_set_uniforms(light_sun_t* light_sun, shader_t* shader) {
    shader_set_int(shader, "u_shadow_map", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, light_sun->shadow_map.texture);
    shader_set_mat4(shader, "u_light_view_projection", light_sun->light_view_projection);
}
