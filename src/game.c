#include "game.h"
#include "assets.h"
#include "asset_data.h"
#include "camera.h"
#include "globals.h"
#include "log.h"
#include "mesh.h"
#include "physics.h"
#include "player.h"
#include "shader.h"
#include "ui.h"
#include <cglm/affine-pre.h>
#include <cglm/cam.h>

// Disable clang-format for this block
// clang-format off

vertex_t plain_axes_vertices[] = {
    { { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
    { { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
};

u32 plain_axes_indices[] = {
    0, 1, 2, 3, 4, 5,
};

vertex_t cube_skeleton_vertices[] = {
    // Back
    { { -0.505f, -0.505f, -0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.505f, -0.505f, -0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.505f,  0.505f, -0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.505f,  0.505f, -0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    // Front
    { { -0.505f, -0.505f, 0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.505f, -0.505f, 0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.505f,  0.505f, 0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.505f,  0.505f, 0.505f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
};

u32 cube_skeleton_indices[] = {
    0, 1, 1, 3, 3, 2, 2, 0, // Back
    4, 5, 5, 7, 7, 6, 6, 4, // Front
    0, 4, 1, 5, 3, 7, 2, 6, // Sides
};

vertex_t cube_vertices[] = {
    // Back
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BL(0, 0) },
    { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BR(0, 0) },
    { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_TL(0, 0) },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_TR(0, 0) },
    // Front
    { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_BL(0, 0) },
    { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_BR(0, 0) },
    { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TL(0, 0) },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TR(0, 0) },
    // Left
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BL(0, 0) },
    { { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_BR(0, 0) },
    { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_TL(0, 0) },
    { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TR(0, 0) },
    // Right
    { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BL(0, 0) },
    { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_BR(0, 0) },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_TL(0, 0) },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TR(0, 0) },
    // Bottom
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BL(0, 0) },
    { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BR(0, 0) },
    { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TL(0, 0) },
    { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TR(0, 0) },
    // Top
    { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BL(0, 0) },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f }, ATLAS_TEXTURE_SLOT_UV_BR(0, 0) },
    { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TL(0, 0) },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f }, ATLAS_TEXTURE_SLOT_UV_TR(0, 0) },
};

// for GL_TRIANGLES
u32 cube_indices[] = {
    0, 1, 2, 2, 1, 3,
    // front
    4, 6, 5, 5, 6, 7,
    // left
    8, 10, 9, 9, 10, 11,
    // right
    12, 13, 14, 14, 13, 15,
    // bottom
    16, 18, 17, 17, 18, 19,
    // top
    20, 21, 22, 22, 21, 23,
};

vertex_t magic_plane_vertices[] = {
    { { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
    { { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
    { { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
};

u32 magic_plane_indices[] = {
    0, 1, 2, 2, 1, 3,
};

// clang-format on

int game_load_content(void) {
    g_game.content.plain_axes = (mesh_t){
        .draw_mode = GL_LINES,
        .vertex_count = 6,
        .index_count = 6,
        .vertices = plain_axes_vertices,
        .indices = plain_axes_indices,
    };

    mesh_init(&g_game.content.plain_axes);

    g_game.content.cube_skeleton = (mesh_t){
        .draw_mode = GL_LINES,
        .vertex_count = 8,
        .index_count = 24,
        .vertices = cube_skeleton_vertices,
        .indices = cube_skeleton_indices,
    };

    mesh_init(&g_game.content.cube_skeleton);

    g_game.content.quad = (mesh_t){
        .draw_mode = GL_TRIANGLES,
        .vertex_count = 4,
        .index_count = 6,
        .vertices = magic_plane_vertices,
        .indices = magic_plane_indices,
    };

    mesh_init(&g_game.content.quad);

    LOG_INFO("Meshes initialized\n");

    texture_t atlas =
        texture_load_from_memory(a_asset_data.textures.atlas, a_asset_data.textures.atlas_len);

    if (!atlas.id) {
        LOG_ERROR("Failed to load texture\n");
        return -1;
    }

    g_game.content.atlas = atlas;

    u8 magic_pixel_data[4] = { 255, 255, 255, 255 };
    g_magic_pixel = texture_load_from_memory_raw(magic_pixel_data, 1, 1, 4);
    if (!g_magic_pixel.id) {
        LOG_ERROR("Failed to create magic pixel\n");
        return -1;
    }

    texture_t cursor = texture_load_from_memory(
        a_asset_data.textures.cursor,
        a_asset_data.textures.cursor_len
    );

    if (!cursor.id) {
        LOG_ERROR("Failed to load cursor texture\n");
        return -1;
    }

    g_game.content.cursor = cursor;

    LOG_INFO("Textures loaded\n");

    shader_t shader = shader_new(a_asset_data.shaders.ui_vert, a_asset_data.shaders.ui_frag);

    g_game.content.ui_shader = shader;
    g_game.content.sprite_shader = shader;

    g_game.content.world_shader =
        shader_new(a_asset_data.shaders.world_vert, a_asset_data.shaders.world_frag);

    g_game.content.gizmo_shader =
        shader_new(a_asset_data.shaders.world_vert, a_asset_data.shaders.gizmo_frag);

    LOG_INFO("Shader initialized\n");

    return 0;
}

int game_init(void) {
    g_game.instances.plain_axes_instance = mesh_instance_new(&g_game.content.plain_axes);

    g_game.instances.cube_skeleton_instance = mesh_instance_new(&g_game.content.cube_skeleton);
    g_game.instances.cube_skeleton_instance.color[0] = 0.0f;
    g_game.instances.cube_skeleton_instance.color[1] = 0.0f;
    g_game.instances.cube_skeleton_instance.color[2] = 0.0f;
    g_game.instances.cube_skeleton_instance.color[3] = 1.0f;

    LOG_INFO("Mesh instances initialized\n");

    ui_init(&g_game.ui);

    ui_element_t crosshair = ui_element_create_image(
        UI_ELEMENT_POSITION_RELATIVE_CC,
        (rect_t){ 0.0, 0.0, 32.0, 32.0 },
        g_game.content.cursor
    );

    ui_element_add_child(&g_game.ui.root, crosshair);

    ui_update(&g_game.ui);

    LOG_INFO("UI initialized\n");

    camera_t camera =
        camera_new((vec3){ 0.0f, 0.0f, 6.0f }, (vec3){ 0.0f, 0.0f, 0.0f }, GLM_MAT4_IDENTITY);

    glm_perspective(
        glm_rad(120.0f),
        (float)g_window_size[0] / (float)g_window_size[1],
        0.1f,
        400.0f,
        camera.projection
    );

    camera_look_at(&camera, (vec3){ 0.0f, 0.0f, 0.0f }, (vec3){ 0.0f, 1.0f, 0.0f });

    LOG_INFO("Camera initialized\n");

    g_player = player_new((vec3){ 0.0f, 0.0f, 0.0f }, (vec3){ 0.0f, 0.0f, 0.0f }, camera);

    LOG_INFO("Player initialized\n");

    g_game.world = world_new();
    for (i32 x = 0; x < 8; x++) {
        for (i32 y = 0; y < 2; y++) {
            for (i32 z = 0; z < 8; z++) {
                world_get_or_load_chunk(g_game.world, (ivec3){ x, y, z });
            }
        }
    }

    LOG_INFO("World initialized\n");

    return 0;
}

void game_update(void) {
    player_update(&g_player);

    g_player.old_selected_block[0] = g_player.selected_block[0];
    g_player.old_selected_block[1] = g_player.selected_block[1];
    g_player.old_selected_block[2] = g_player.selected_block[2];

    g_player.block_selected =
        camera_pointed_block(&g_player.camera, g_game.world, 8.0f, &g_player.selected_block);

    if (g_player.block_selected &&
        (g_player.old_selected_block[0] != g_player.selected_block[0] ||
         g_player.old_selected_block[1] != g_player.selected_block[1] ||
         g_player.old_selected_block[2] != g_player.selected_block[2])) {
        glm_mat4_identity(g_game.instances.cube_skeleton_instance.transform);
        glm_translate(
            g_game.instances.cube_skeleton_instance.transform,
            (vec3){ (float)g_player.selected_block[0] + 0.5f,
                    (float)g_player.selected_block[1] + 0.5f,
                    (float)g_player.selected_block[2] + 0.5f }
        );

        LOG_INFO(
            "Selected block: %d %d %d\n",
            g_player.selected_block[0],
            g_player.selected_block[1],
            g_player.selected_block[2]
        );
    }

    if (g_mouse.buttons_down[0]) {
        world_try_set_block_at(g_game.world, g_player.selected_block, BLOCK_AIR);
    }

    if (g_player.block_selected && g_mouse.buttons[1]) {
        vec3 normal;

        LOG_INFO("Normal: %f %f %f\n", normal[0], normal[1], normal[2]);

        ray_t ray;
        ray_from_camera(&ray, &g_player.camera);

        ray_intersect_block(
            ray,
            g_game.world,
            8.0f,
            BLOCK_FLAG_SOLID,
            &g_player.selected_block,
            &normal
        );

        ivec3 block_to_set = { g_player.selected_block[0] + (i32)normal[0],
                               g_player.selected_block[1] + (i32)normal[1],
                               g_player.selected_block[2] + (i32)normal[2] };

        LOG_INFO("Block to set: %d %d %d\n", block_to_set[0], block_to_set[1], block_to_set[2]);

        world_set_block_at(g_game.world, block_to_set, 4);
    }
}

void game_draw(void) {
    shader_use(&g_game.content.world_shader);
    player_set_uniforms(&g_player, &g_game.content.world_shader);

    shader_set_vec3(&g_game.content.world_shader, "u_light_dir", (vec3){ 1.0f, 1.0f, 1.0f });

    shader_set_vec4(
        &g_game.content.world_shader,
        "u_ambient_color",
        (vec4){ 0.3f, 0.3f, 0.3f, 1.0f }
    );

    shader_set_vec4(
        &g_game.content.world_shader,
        "u_fog_color",
        (vec4){ 0.5f, 0.8f, 1.0f, 1.0f }
    );

    shader_set_vec4(&g_game.content.world_shader, "u_color", (vec4){ 1.0f, 1.0f, 1.0f, 1.0f });

    shader_set_float(&g_game.content.world_shader, "u_fog_start", 0.0f);
    shader_set_float(&g_game.content.world_shader, "u_fog_end", 200.0f);
    shader_set_float(&g_game.content.world_shader, "u_fog_density", 0.0001f);

    if (g_debug_tools.no_textures) {
        texture_bind(&g_magic_pixel, 0);
    } else {
        texture_bind(&g_game.content.atlas, 0);
    }

    world_draw(g_game.world);

    shader_use(&g_game.content.gizmo_shader);
    player_set_uniforms(&g_player, &g_game.content.gizmo_shader);

    shader_set_float(&g_game.content.gizmo_shader, "u_fog_start", 0.0f);
    shader_set_float(&g_game.content.gizmo_shader, "u_fog_end", 200.0f);
    shader_set_float(&g_game.content.gizmo_shader, "u_fog_density", 0.0001f);

    mesh_instance_draw(&g_game.instances.plain_axes_instance);

    if (g_player.block_selected) {
        mesh_instance_draw(&g_game.instances.cube_skeleton_instance);
    }
}

void game_draw_debug(void) {}

void game_draw_ui(void) {
    ui_draw(&g_game.ui);
}

void game_free(void) {
    mesh_free(&g_game.content.cube_skeleton);
    mesh_free(&g_game.content.plain_axes);

    shader_free(&g_game.content.world_shader);
    shader_free(&g_game.content.ui_shader);
    // shader_free(&g_game.content.sprite_shader);

    texture_free(&g_game.content.atlas);
    texture_free(&g_magic_pixel);
    texture_free(&g_game.content.cursor);

    world_free(g_game.world);
    ui_free(&g_game.ui);
}
