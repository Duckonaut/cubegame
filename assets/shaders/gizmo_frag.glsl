#version 330 core

out vec4 o_fragColor;

in vec3 m_world_pos;

uniform vec4 u_color;
uniform vec3 u_world_eye;

uniform vec4 u_fog_color;
uniform float u_fog_start;
uniform float u_fog_end;
uniform float u_fog_density;

float fog_factor(float dist) {
    // exponential fog
    return 1.0 - clamp(exp(-u_fog_density * dist * dist), 0.0, 1.0);
}

void main() {
    float dist = length(m_world_pos - u_world_eye);
    float fogFactor = fog_factor(dist);

    // apply fog
    o_fragColor = u_color;
}

