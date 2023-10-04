#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in uint a_shadow_index;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

// shadows
uniform uint u_grid_map[512]; // 16x16x16 / 4 / 2 (uint = 4 bytes, we need half a byte per voxel)

out vec3 m_normal;
out vec2 m_uv;
out vec3 m_world_pos;
out float m_shadow;

uint grid_value(uint shadow_index) {
    uint packed_index = shadow_index / 8u;
    uint grid_value = u_grid_map[packed_index];
    uint unpacked_value = (grid_value >> ((uint(shadow_index) % 8u) * 4u)) & 0xFu;
    return unpacked_value;
}

void main()
{
    mat4 mvp = u_projection * u_view * u_model;
    gl_Position = mvp * vec4(a_pos, 1.0f);
    m_world_pos = (u_model * vec4(a_pos, 1.0f)).xyz;
    m_normal = mat3(transpose(inverse(u_model))) * a_normal;
    m_uv = a_uv;
    m_shadow = float(grid_value(a_shadow_index)) / 15.0f;
}
