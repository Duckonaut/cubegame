#pragma once

#include "assets.h"
#include "mesh.h"
#include <cglm/types.h>

typedef enum ui_element_type {
    UI_ELEMENT_TYPE_BOX,
    UI_ELEMENT_TYPE_BUTTON,
    UI_ELEMENT_TYPE_TEXT,
    UI_ELEMENT_TYPE_IMAGE,
} ui_element_type_t;

typedef enum ui_element_position {
    UI_ELEMENT_POSITION_ABSOLUTE,
    UI_ELEMENT_POSITION_RELATIVE_TL,
    UI_ELEMENT_POSITION_RELATIVE_TR,
    UI_ELEMENT_POSITION_RELATIVE_BL,
    UI_ELEMENT_POSITION_RELATIVE_BR,
    UI_ELEMENT_POSITION_RELATIVE_CC,
    UI_ELEMENT_POSITION_RELATIVE_CL,
    UI_ELEMENT_POSITION_RELATIVE_CR,
    UI_ELEMENT_POSITION_RELATIVE_TC,
    UI_ELEMENT_POSITION_RELATIVE_BC,
} ui_element_position_t;

typedef struct rect {
    float x;
    float y;
    float width;
    float height;
} rect_t;

typedef struct ui_element {
    ui_element_position_t position_style;
    rect_t pos;
    rect_t _real_pos;

    ui_element_type_t type;
    union {
        struct {
            texture_t texture;
            mesh_instance_t instance;
        } image;
        struct {
            texture_t texture;
            mesh_instance_t instance;
            void (*on_click)(void);
        } button;
        struct {
            char* text;
        } text;
    };

    usize children_count;
    usize children_capacity;
    struct ui_element* children;
    struct ui_element* parent;
} ui_element_t;

typedef struct ui {
    ui_element_t root;
    mat4 projection;
} ui_t;

void ui_init(ui_t* ui);

void ui_update(ui_t* ui);

void ui_draw(ui_t* ui);

void ui_free(ui_t* ui);

ui_element_t ui_element_create_box(ui_element_position_t position_style, rect_t pos);

ui_element_t ui_element_create_button(
    ui_element_position_t position_style,
    rect_t pos,
    texture_t texture,
    void (*on_click)(void)
);

ui_element_t
ui_element_create_text(ui_element_position_t position_style, rect_t pos, const char* text);

ui_element_t
ui_element_create_image(ui_element_position_t position_style, rect_t pos, texture_t texture);

void ui_element_update(ui_element_t* element);

void ui_element_draw(ui_element_t* element);

void ui_element_free(ui_element_t* element);

void ui_element_add_child(ui_element_t* parent, ui_element_t child);

void ui_element_remove_child(ui_element_t* parent, ui_element_t* child);

void ui_element_remove_all_children(ui_element_t* parent);
