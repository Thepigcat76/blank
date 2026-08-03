#pragma once

#include "../blank.h"
#include <stddef.h>
#include <stdbool.h>

#define BLANK_ELEM_BUTTON 0
#define BLANK_ELEM_GROUP 1
#define BLANK_ELEM_IMAGE 2
#define BLANK_ELEM_LABEL 3

typedef struct {
  Blank_UiLayout layout;
  Blank_UiElement *ui_elems;
} Blank_UiElemGroup;

typedef struct {
  const char *text;
  bool disabled;
  Blank_Color text_color;
  Blank_Color bg_color;
} Blank_UiElemButton;

typedef struct {
  const char *img_path;
  bool keep_ratio;
  f32 scale;
} Blank_UiElemImage;

/* UI-Group functions */

void blank_ui_group(Blank_UiElemGroup *group, Blank_UiElement elem);

/* UI-Element creation functions */

Blank_UiElement blank_ui_image(u64 uid, Blank_UiElemImage ui_elem_image);

#define BLANK_IMAGE(uid, ...) blank_ui_image(uid, (Blank_UiElemImage){__VA_ARGS__})

Blank_UiElement blank_ui_button(u64 uid, Blank_UiElemButton ui_elem_button);

#define BLANK_BUTTON(uid, ...) blank_ui_button(uid, (Blank_UiElemButton){__VA_ARGS__})

Blank_UiElement blank_button(const char *text, bool disabled);

Blank_UiElement blank_group(Blank_UiElemGroup group);
