#version 330 core

out vec4 o_fragColor;

in vec3 m_normal;
in vec2 m_uv;
in vec3 m_world_pos;

uniform sampler2D u_texture;
uniform vec4 u_color;
uniform vec3 u_world_eye;

uniform vec4 u_fog_color;
uniform float u_fog_start;
uniform float u_fog_end;
uniform float u_fog_density;

uniform vec3 u_light_dir;
uniform float u_light_intensity;
uniform vec4 u_ambient_color;

float fog_factor(float dist) {
    // exponential fog
    return 1.0 - clamp(exp(-u_fog_density * dist * dist), 0.0, 1.0);
}

void main() {
    vec4 texColor = texture2D(u_texture, m_uv);
    float dist = length(m_world_pos - u_world_eye);
    float fogFactor = fog_factor(dist);

    // calulate normal-based lighting
    float diffuse = max(dot(m_normal, u_light_dir), 0.0) * u_light_intensity;
    vec4 diffuseColor = vec4(vec3(diffuse), 1.0);

    // calculate final color
    vec4 finalColor = (diffuseColor + u_ambient_color) * texColor;

    // apply fog
    o_fragColor = mix(finalColor, u_fog_color, fogFactor) * u_color;
}
