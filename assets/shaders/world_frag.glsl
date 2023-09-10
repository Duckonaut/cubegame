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

const vec2 c_poisson_values[9] = vec2[](
    vec2(-0.326212, -0.40581),
    vec2(-0.840144, -0.07358),
    vec2(-0.695914, 0.457137),
    vec2(-0.203345, 0.620716),
    vec2(0.96234, -0.194983),
    vec2(0.473434, -0.480026),
    vec2(0.519456, 0.767022),
    vec2(0.185461, -0.893124),
    vec2(0.507431, 0.064425)
);

float fog_factor(float dist) {
    // exponential fog
    return 1.0 - clamp(exp(-u_fog_density * dist * dist), 0.0, 1.0);
}

float random(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float shadow_factor(vec4 light_space_pos) {
    vec3 shadow_pos = light_space_pos.xyz / light_space_pos.w;
    shadow_pos = shadow_pos * 0.5 + 0.5;

    float bias = 0.0005 * tan(acos(dot(m_normal, u_light_dir)));
    float noise_offset = 1.0 / 5000.0;
    // use random offset from poisson disk
    float shadow = 0.0;
    for (int i = 0; i < 9; i++) {
        int index = int(random(m_uv + c_poisson_values[i]) * 500.0);
        vec3 sample_pos = shadow_pos + vec3(c_poisson_values[index % 9] * noise_offset, 0.0);
        float depth = texture(u_shadow_map, sample_pos);
        if (depth < shadow_pos.z - bias) {
            shadow += 1.0;
        }
    }

    return shadow / 9.0;
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
    finalColor.rgb *= 1.0 - shadow * (1.0 - u_ambient_color.rgb);

    // apply fog
    o_fragColor = mix(finalColor, u_fog_color, fogFactor);
}
