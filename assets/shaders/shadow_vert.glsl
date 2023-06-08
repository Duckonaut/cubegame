#version 330 core

layout (location = 0) in vec3 a_pos;

uniform mat4 u_light_view_projection;
uniform mat4 u_model;

out vec4 m_world_pos;

void main()
{
    gl_Position = u_light_view_projection * u_model * vec4(a_pos, 1.0);
}
