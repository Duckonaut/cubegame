#pragma once

#include <cglm/types.h>
#include <stdbool.h>
#define VEC3_ZERO                                                                              \
    (vec3) {                                                                                   \
        0.0f, 0.0f, 0.0f                                                                       \
    }

#define VEC3_ONE                                                                               \
    (vec3) {                                                                                   \
        1.0f, 1.0f, 1.0f                                                                       \
    }

#define VEC3_UP                                                                                \
    (vec3) {                                                                                   \
        0.0f, 1.0f, 0.0f                                                                       \
    }

#define VEC3_DOWN                                                                              \
    (vec3) {                                                                                   \
        0.0f, -1.0f, 0.0f                                                                      \
    }

#define VEC3_RIGHT                                                                             \
    (vec3) {                                                                                   \
        1.0f, 0.0f, 0.0f                                                                       \
    }

#define VEC3_LEFT                                                                              \
    (vec3) {                                                                                   \
        -1.0f, 0.0f, 0.0f                                                                      \
    }

#define VEC3_FORWARD                                                                           \
    (vec3) {                                                                                   \
        0.0f, 0.0f, -1.0f                                                                      \
    }

#define VEC3_BACK                                                                              \
    (vec3) {                                                                                   \
        0.0f, 0.0f, 1.0f                                                                       \
    }

#define IVEC3_ZERO                                                                             \
    (ivec3) {                                                                                  \
        0, 0, 0                                                                                \
    }

#define IVEC3_ONE                                                                              \
    (ivec3) {                                                                                  \
        1, 1, 1                                                                                \
    }

#define IVEC3_UP                                                                               \
    (ivec3) {                                                                                  \
        0, 1, 0                                                                                \
    }

#define IVEC3_DOWN                                                                             \
    (ivec3) {                                                                                  \
        0, -1, 0                                                                               \
    }

#define IVEC3_RIGHT                                                                            \
    (ivec3) {                                                                                  \
        1, 0, 0                                                                                \
    }

#define IVEC3_LEFT                                                                             \
    (ivec3) {                                                                                  \
        -1, 0, 0                                                                               \
    }

#define IVEC3_FORWARD                                                                          \
    (ivec3) {                                                                                  \
        0, 0, -1                                                                               \
    }

#define IVEC3_BACK                                                                             \
    (ivec3) {                                                                                  \
        0, 0, 1                                                                                \
    }

static inline bool glme_ivec3_eq(ivec3 a, ivec3 b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}
