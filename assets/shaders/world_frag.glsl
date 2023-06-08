#version 330 core

out vec4 o_fragColor;

in vec3 m_normal;
in vec2 m_uv;
in vec3 m_world_pos;
in vec4 m_light_space_pos;

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

// shadows
uniform mat4 u_light_view_projection;
uniform sampler2DShadow u_shadow_map;

float fog_factor(float dist) {
    // exponential fog
    return 1.0 - clamp(exp(-u_fog_density * dist * dist), 0.0, 1.0);
}

float shadow_factor(vec4 light_space_pos) {
    vec3 shadow_pos = light_space_pos.xyz / light_space_pos.w;
    shadow_pos = shadow_pos * 0.5 + 0.5;

    float shadow_depth = texture(u_shadow_map, shadow_pos);
    float current_depth = shadow_pos.z;

    float bias = 0.001;

    float shadow = current_depth - bias > shadow_depth ? 1.0 : 0.0;

    return shadow;
}

void main() {
    vec4 texColor = texture2D(u_texture, m_uv);
    float dist = length(m_world_pos - u_world_eye);
    float fogFactor = fog_factor(dist);

    // calulate normal-based lighting
    float diffuse = max(dot(m_normal, u_light_dir), 0.0) * u_light_intensity;
    vec4 diffuseColor = vec4(vec3(diffuse), 1.0);

    // calculate shadow
    float shadow = shadow_factor(m_light_space_pos);

    // calculate final color
    vec4 finalColor = (diffuseColor + u_ambient_color) * texColor;

    // apply shadow
    finalColor.rgb *= 1.0 - shadow * 0.8;

    // apply fog
    o_fragColor = mix(finalColor, u_fog_color * (1.0 - shadow * 0.8), fogFactor) * u_color;
}
