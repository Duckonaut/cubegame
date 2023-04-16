#version 330 core

out vec4 o_fragColor;

in vec3 m_color;
in vec2 m_uv;

uniform sampler2D u_texture;
uniform vec4 u_color;

void main() {
    vec3 texColor = texture2D(u_texture, m_uv).rgb;
    o_fragColor = vec4(texColor * u_color.rgb, 1.0 * u_color.a);
}
