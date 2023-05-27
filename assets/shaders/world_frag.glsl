#version 330 core

out vec4 o_fragColor;

in vec3 m_color;
in vec2 m_uv;
in vec3 m_world_pos;

uniform sampler2D u_texture;
uniform vec4 u_color;
uniform vec3 u_world_eye;

float fog_factor(float dist) {
    float fogStart = 0.0;
    float fogEnd = 100.0;
    float fogFactor = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    return fogFactor;
}

void main() {
    vec4 texColor = texture2D(u_texture, m_uv);
    float dist = length(m_world_pos - u_world_eye);
    float fogFactor = fog_factor(dist);
    vec4 fogColor = vec4(0.0, 0.0, 0.0, 1.0);
    texColor = mix(texColor, fogColor, fogFactor);

    o_fragColor = texColor * u_color;
}
