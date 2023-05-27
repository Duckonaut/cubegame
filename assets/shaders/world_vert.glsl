#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_color;
layout (location = 2) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 m_color;
out vec2 m_uv;
out vec3 m_world_pos;

void main()
{
    mat4 mvp = u_projection * u_view * u_model;
    gl_Position = mvp * vec4(a_pos, 1.0f);
    m_world_pos = (u_model * vec4(a_pos, 1.0f)).xyz;
    m_color = a_color;
    m_uv = a_uv;
}
