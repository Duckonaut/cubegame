#include "types.h"

f32 perlin2d(f32 x, f32 y);

f32 perlin3d(f32 x, f32 y, f32 z);

inline static int posmod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}
