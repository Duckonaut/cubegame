#version 330 core

out vec4 o_fragColor;

in vec3 m_normal;
in vec2 m_uv;
in vec3 m_world_pos;

uniform sampler2D u_texture;
uniform vec4 u_color;

void main() {
    vec4 texColor = texture2D(u_texture, m_uv);

    o_fragColor = texColor * u_color;
}

