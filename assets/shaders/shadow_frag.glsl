#version 330 core
// shadow map fragment shader

out vec4 o_fragColor;

in vec4 m_world_pos;

void main() {
    o_fragColor = vec4(m_world_pos.z, 0.0, 0.0, 1.0);
}
