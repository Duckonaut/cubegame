#include "types.h"
#include <string.h>

f32 perlin2d(f32 x, f32 y);

f32 perlin3d(f32 x, f32 y, f32 z);

inline static int posmod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

inline static bool
array_any_eq(const void* arr, const size_t len, const size_t elem_size, const void* elem) {
    const u8* arr_u8 = arr;
    for (size_t i = 0; i < len; i++) {
        if (memcmp(arr_u8 + i * elem_size, elem, elem_size) == 0) {
            return true;
        }
    }
    return false;
}

inline static bool array_any_member_eq(
    const void* arr,
    const size_t len,
    const size_t full_elem_size,
    const void* member,
    const size_t member_offset,
    const size_t member_len
) {
    const u8* arr_u8 = arr;
    for (size_t i = 0; i < len; i++) {
        if (memcmp(arr_u8 + i * full_elem_size + member_offset, member, member_len) == 0) {
            return true;
        }
    }
    return false;
}
