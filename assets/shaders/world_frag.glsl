#version 330 core

out vec4 o_fragColor;

in vec3 m_normal;
in vec2 m_uv;
in vec3 m_world_pos;
in float m_shadow;

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

float random(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec4 texColor = texture2D(u_texture, m_uv);
    float dist = length(m_world_pos - u_world_eye);
    float fogFactor = fog_factor(dist);

    // apply shadow
    vec4 finalColor = u_ambient_color + m_shadow * (1.0 - u_ambient_color);
    finalColor.rgb *= texColor.rgb * u_color.rgb;
    finalColor.a = texColor.a * u_color.a;

    // apply fog
    o_fragColor = mix(finalColor, u_fog_color, fogFactor);
}
