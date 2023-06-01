#include "utils.h"

#include "types.h"
#include <math.h>
#include <cglm/util.h>

static i32 g_perm[] = {
    208, 34,  231, 213, 32,  248, 233, 56,  161, 78,  24,  140, 71,  48,  140, 254, 245, 255,
    247, 247, 40,  185, 248, 251, 245, 28,  124, 204, 204, 76,  36,  1,   107, 28,  234, 163,
    202, 224, 245, 128, 167, 204, 9,   92,  217, 54,  239, 174, 173, 102, 193, 189, 190, 121,
    100, 108, 167, 44,  43,  77,  180, 204, 8,   81,  70,  223, 11,  38,  24,  254, 210, 210,
    177, 32,  81,  195, 243, 125, 8,   169, 112, 32,  97,  53,  195, 13,  203, 9,   47,  104,
    125, 117, 114, 124, 165, 203, 181, 235, 193, 206, 70,  180, 174, 0,   167, 181, 41,  164,
    30,  116, 127, 198, 245, 146, 87,  224, 149, 206, 57,  4,   192, 210, 65,  210, 129, 240,
    178, 105, 228, 108, 245, 148, 140, 40,  35,  195, 38,  58,  65,  207, 215, 253, 65,  85,
    208, 76,  62,  3,   237, 55,  89,  232, 50,  217, 64,  244, 157, 199, 121, 252, 90,  17,
    212, 203, 149, 152, 140, 187, 234, 177, 73,  174, 193, 100, 192, 143, 97,  53,  145, 135,
    19,  103, 13,  90,  135, 151, 199, 91,  239, 247, 33,  39,  145, 101, 120, 99,  3,   186,
    86,  99,  41,  237, 203, 111, 79,  220, 135, 158, 42,  30,  154, 120, 67,  87,  167, 135,
    176, 183, 191, 253, 115, 184, 21,  233, 58,  129, 233, 142, 39,  128, 211, 118, 137, 139,
    255, 114, 20,  218, 113, 154, 27,  127, 246, 250, 1,   8,   198, 250, 209, 92,  222, 173,
    21,  88,  102, 219
};

static f32 fade(f32 t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static f32 grad2d(i32 hash, f32 x, f32 y) {
    switch (hash & 0x7) {
        case 0x0:
            return x + y;
        case 0x1:
            return -x + y;
        case 0x2:
            return x - y;
        case 0x3:
            return -x - y;
        case 0x4:
            return x;
        case 0x5:
            return -x;
        case 0x6:
            return y;
        case 0x7:
            return -y;
        default:
            return 0.0f;
    }
}

static f32 grad3d(i32 hash, f32 x, f32 y, f32 z) {
    switch (hash & 0xF) {
        case 0x0:
            return x + y;
        case 0x1:
            return -x + y;
        case 0x2:
            return x - y;
        case 0x3:
            return -x - y;
        case 0x4:
            return x + z;
        case 0x5:
            return -x + z;
        case 0x6:
            return x - z;
        case 0x7:
            return -x - z;
        case 0x8:
            return y + z;
        case 0x9:
            return -y + z;
        case 0xA:
            return y - z;
        case 0xB:
            return -y - z;
        case 0xC:
            return y + x;
        case 0xD:
            return -y + z;
        case 0xE:
            return y - x;
        case 0xF:
            return -y - z;
        default:
            return 0.0f;
    }
}

f32 perlin2d(f32 x, f32 y) {
    if (x < 0) x = -x;
    if (y < 0) y = -y;

    i32 xi = posmod((i32)x, 256);
    i32 yi = posmod((i32)y, 256);

    f32 xf = x - floorf(x);
    f32 yf = y - floorf(y);

    f32 u = fade(xf);
    f32 v = fade(yf);

    i32 aa = g_perm[(g_perm[xi] + yi) % 256];
    i32 ab = g_perm[(g_perm[xi] + yi + 1) % 256];
    i32 ba = g_perm[(g_perm[(xi + 1) % 256] + yi) % 256];
    i32 bb = g_perm[(g_perm[(xi + 1) % 256] + yi + 1) % 256];

    f32 x1 = glm_lerp(grad2d(g_perm[aa], xf, yf), grad2d(g_perm[ba], xf - 1, yf), u);
    f32 x2 = glm_lerp(grad2d(g_perm[ab], xf, yf - 1), grad2d(g_perm[bb], xf - 1, yf - 1), u);

    return (glm_lerp(x1, x2, v) + 1) / 2;
}

f32 perlin3d(f32 x, f32 y, f32 z) {
    i32 xi = (i32)x & 255;
    i32 yi = (i32)y & 255;
    i32 zi = (i32)z & 255;

    f32 xf = x - floorf(x);
    f32 yf = y - floorf(y);
    f32 zf = z - floorf(z);

    f32 u = fade(xf);
    f32 v = fade(yf);
    f32 w = fade(zf);

    i32 aaa = g_perm[g_perm[g_perm[xi] + yi] + zi];
    i32 aba = g_perm[g_perm[g_perm[xi] + yi + 1] + zi];
    i32 aab = g_perm[g_perm[g_perm[xi] + yi] + zi + 1];
    i32 abb = g_perm[g_perm[g_perm[xi] + yi + 1] + zi + 1];
    i32 baa = g_perm[g_perm[g_perm[xi + 1] + yi] + zi];
    i32 bba = g_perm[g_perm[g_perm[xi + 1] + yi + 1] + zi];
    i32 bab = g_perm[g_perm[g_perm[xi + 1] + yi] + zi + 1];
    i32 bbb = g_perm[g_perm[g_perm[xi + 1] + yi + 1] + zi + 1];

    f32 x1 = glm_lerp(grad3d(g_perm[aaa], xf, yf, zf), grad3d(g_perm[baa], xf - 1, yf, zf), u);
    f32 x2 = glm_lerp(
        grad3d(g_perm[aba], xf, yf - 1, zf),
        grad3d(g_perm[bba], xf - 1, yf - 1, zf),
        u
    );
    f32 y1 = glm_lerp(x1, x2, v);

    x1 = glm_lerp(
        grad3d(g_perm[aab], xf, yf, zf - 1),
        grad3d(g_perm[bab], xf - 1, yf, zf - 1),
        u
    );
    x2 = glm_lerp(
        grad3d(g_perm[abb], xf, yf - 1, zf - 1),
        grad3d(g_perm[bbb], xf - 1, yf - 1, zf - 1),
        u
    );
    f32 y2 = glm_lerp(x1, x2, v);

    return (glm_lerp(y1, y2, w) + 1) / 2;
}
