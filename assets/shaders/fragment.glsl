#version 330 core

out vec4 o_fragColor;

in vec3 m_color;

void main() {
    o_fragColor = vec4(m_color, 1.0);
}
