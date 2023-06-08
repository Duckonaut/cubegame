#pragma once

#include "shader.h"
#include "types.h"

#include <GL/glew.h>
#include <cglm/types.h>

#define SHADOW_MAP_SIZE 8192

// shadow map framebuffer
typedef struct shadow_map {
    GLuint fbo;
    GLuint texture;
    GLuint depth;
} shadow_map_t;

void shadow_map_init(shadow_map_t* shadow_map);

void shadow_map_bind(shadow_map_t* shadow_map);

void shadow_map_unbind(void);

void shadow_map_free(shadow_map_t* shadow_map);

typedef struct light_sun {
    vec3 position;
    vec3 direction;
    vec3 color;
    f32 intensity;
    mat4 light_view_projection;
    shadow_map_t shadow_map;
} light_sun_t;

void light_sun_init(
    light_sun_t* light_sun,
    vec3 position,
    vec3 direction,
    vec3 color,
    f32 intensity
);

void light_sun_free(light_sun_t* light_sun);

void light_sun_shadow_update(light_sun_t* light_sun);

void light_sun_shadow_set_uniforms(light_sun_t* light_sun, shader_t* shader);
