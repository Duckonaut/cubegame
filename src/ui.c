#include "ui.h"

#include "assets.h"
#include "game.h"
#include "globals.h"
#include "mesh.h"
#include "types.h"
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <stdlib.h>
#include <string.h>

game_state_t g_game;

void ui_init(ui_t* ui) {
    ui_element_t screen_box = ui_element_create_box(
        UI_ELEMENT_POSITION_ABSOLUTE,
        (rect_t){ 0, 0, (float)g_window_size[0], (float)g_window_size[1] }
    );

    ui->root = screen_box;
}

void ui_update(ui_t* ui) {
    ui->root.pos.width = (float)g_window_size[0];
    ui->root.pos.height = (float)g_window_size[1];
    ui_element_update(&ui->root);
}

void ui_draw(ui_t* ui) {
    ui_element_draw(&ui->root);
}

void ui_free(ui_t* ui) {
    ui_element_free(&ui->root);
}

ui_element_t ui_element_create_box(ui_element_position_t position_style, rect_t pos) {
    ui_element_t element = {
        .position_style = position_style,
        .pos = pos,
        .type = UI_ELEMENT_TYPE_BOX,
        .children_count = 0,
        .children_capacity = 0,
        .children = NULL,
        .parent = NULL,
    };

    return element;
}

ui_element_t
ui_element_create_text(ui_element_position_t position_style, rect_t pos, const char* text) {
    char* text_copy = malloc(strlen(text) + 1);

    ui_element_t element = {
        .position_style = position_style,
        .pos = pos,
        .type = UI_ELEMENT_TYPE_TEXT,
        .text.text = text_copy,
        .children_count = 0,
        .children_capacity = 0,
        .children = NULL,
        .parent = NULL,
    };

    return element;
}

ui_element_t
ui_element_create_image(ui_element_position_t position_style, rect_t pos, texture_t texture) {
    mesh_instance_t instance = mesh_instance_new(&g_game.content.quad);

    ui_element_t element = {
        .position_style = position_style,
        .pos = pos,
        .type = UI_ELEMENT_TYPE_IMAGE,
        .image.texture = texture,
        .image.instance = instance,
        .children_count = 0,
        .children_capacity = 0,
        .children = NULL,
        .parent = NULL,
    };

    return element;
}

ui_element_t ui_element_create_button(
    ui_element_position_t position_style,
    rect_t pos,
    texture_t texture,
    void (*on_click)(void)
) {
    mesh_instance_t instance = mesh_instance_new(&g_game.content.quad);

    ui_element_t element = {
        .position_style = position_style,
        .pos = pos,
        .type = UI_ELEMENT_TYPE_BUTTON,
        .button.texture = texture,
        .button.instance = instance,
        .button.on_click = on_click,
        .children_capacity = 0,
        .children = NULL,
        .parent = NULL,
    };

    return element;
}

void ui_element_update(ui_element_t* element) {
    ui_element_t* parent = element->parent;
    if (parent == NULL) {
        element->_real_pos = element->pos;
    } else {
        switch (element->position_style) {
            case UI_ELEMENT_POSITION_RELATIVE_TL:
                element->_real_pos = (rect_t){
                    parent->_real_pos.x + element->pos.x,
                    parent->_real_pos.y + element->pos.y,
                    element->pos.width,
                    element->pos.height,
                };
                break;
            case UI_ELEMENT_POSITION_RELATIVE_TR:
                element->_real_pos = (rect_t){
                    parent->_real_pos.x + parent->_real_pos.width - element->pos.width -
                        element->pos.x,
                    parent->_real_pos.y + element->pos.y,
                    element->pos.width,
                    element->pos.height,
                };
                break;
            case UI_ELEMENT_POSITION_RELATIVE_BL:
                element->_real_pos = (rect_t){
                    parent->_real_pos.x + element->pos.x,
                    parent->_real_pos.y + parent->_real_pos.height - element->pos.height -
                        element->pos.y,
                    element->pos.width,
                    element->pos.height,
                };
                break;
            case UI_ELEMENT_POSITION_RELATIVE_BR:
                element->_real_pos = (rect_t){
                    parent->_real_pos.x + parent->_real_pos.width - element->pos.width -
                        element->pos.x,
                    parent->_real_pos.y + parent->_real_pos.height - element->pos.height -
                        element->pos.y,
                    element->pos.width,
                    element->pos.height,
                };
                break;
            case UI_ELEMENT_POSITION_RELATIVE_CC:
                element->_real_pos = (rect_t){
                    parent->_real_pos.x + parent->_real_pos.width / 2 - element->pos.width / 2 +
                        element->pos.x,
                    parent->_real_pos.y + parent->_real_pos.height / 2 -
                        element->pos.height / 2 + element->pos.y,
                    element->pos.width,
                    element->pos.height,
                };
                break;
            case UI_ELEMENT_POSITION_RELATIVE_CL:
                element->_real_pos = (rect_t){
                    parent->_real_pos.x + element->pos.x,
                    parent->_real_pos.y + parent->_real_pos.height / 2 -
                        element->pos.height / 2 + element->pos.y,
                    element->pos.width,
                    element->pos.height,
                };
                break;
            case UI_ELEMENT_POSITION_RELATIVE_CR:
                element->_real_pos = (rect_t){
                    parent->_real_pos.x + parent->_real_pos.width - element->pos.width -
                        element->pos.x,
                    parent->_real_pos.y + parent->_real_pos.height / 2 -
                        element->pos.height / 2 + element->pos.y,
                    element->pos.width,
                    element->pos.height,
                };
                break;
            case UI_ELEMENT_POSITION_RELATIVE_TC:
                element->_real_pos = (rect_t){
                    parent->_real_pos.x + parent->_real_pos.width / 2 - element->pos.width / 2 +
                        element->pos.x,
                    parent->_real_pos.y + element->pos.y,
                    element->pos.width,
                    element->pos.height,
                };
                break;
            case UI_ELEMENT_POSITION_RELATIVE_BC:
                element->_real_pos = (rect_t){
                    parent->_real_pos.x + parent->_real_pos.width / 2 - element->pos.width / 2 +
                        element->pos.x,
                    parent->_real_pos.y + parent->_real_pos.height - element->pos.height -
                        element->pos.y,
                    element->pos.width,
                    element->pos.height,
                };
                break;
            default:
                element->_real_pos = element->pos;
                break;
        }
    }

    switch (element->type) {
        case UI_ELEMENT_TYPE_TEXT:
            break;
        case UI_ELEMENT_TYPE_IMAGE:
            glm_mat4_identity(element->image.instance.transform);
            glm_translate(
                element->image.instance.transform,
                (vec3){ element->_real_pos.x, element->_real_pos.y, 0.0f }
            );
            glm_scale(
                element->image.instance.transform,
                (vec3){ element->_real_pos.width, element->_real_pos.height, 1.0f }
            );
            break;
        case UI_ELEMENT_TYPE_BUTTON:
            glm_mat4_identity(element->button.instance.transform);
            glm_translate(
                element->button.instance.transform,
                (vec3){ element->_real_pos.x, element->_real_pos.y, 0.0f }
            );
            glm_scale(
                element->button.instance.transform,
                (vec3){ element->_real_pos.width, element->_real_pos.height, 1.0f }
            );
            break;
        default:
            break;
    }

    for (size_t i = 0; i < element->children_count; ++i) {
        ui_element_update(&element->children[i]);
    }
}

void ui_element_draw(ui_element_t* element) {
    switch (element->type) {
        case UI_ELEMENT_TYPE_TEXT:
            break;
        case UI_ELEMENT_TYPE_IMAGE:
            texture_bind(&element->image.texture, 0);
            mesh_instance_draw(&element->image.instance);
            break;
        case UI_ELEMENT_TYPE_BUTTON:
            texture_bind(&element->button.texture, 0);
            mesh_instance_draw(&element->button.instance);
            break;
        default:
            break;
    }

    for (size_t i = 0; i < element->children_count; ++i) {
        ui_element_draw(&element->children[i]);
    }
}

void ui_element_add_child(ui_element_t* parent, ui_element_t child) {
    if (parent->children_count >= parent->children_capacity) {
        if (parent->children_capacity == 0) {
            parent->children_capacity = 4;
            parent->children = malloc(sizeof(ui_element_t) * parent->children_capacity);
        } else {
            parent->children_capacity *= 2;
            parent->children =
                realloc(parent->children, sizeof(ui_element_t) * parent->children_capacity);
        }
    }

    child.parent = parent;
    parent->children[parent->children_count++] = child;
}

void ui_element_remove_child(ui_element_t* parent, ui_element_t* child) {
    for (size_t i = 0; i < parent->children_count; ++i) {
        if (&parent->children[i] == child) {
            parent->children_count--;
            memmove(&parent->children[i], &parent->children[i + 1], parent->children_count - i);
            break;
        }
    }
}

void ui_element_remove_all_children(ui_element_t* parent) {
    parent->children_count = 0;
}

void ui_element_free(ui_element_t *element) {
    for (size_t i = 0; i < element->children_count; ++i) {
        ui_element_free(&element->children[i]);
    }

    if (element->children_capacity > 0) {
        free(element->children);
    }
}
