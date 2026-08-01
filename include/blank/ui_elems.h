#pragma once

#include "../blank.h"
#include <stddef.h>
#include <stdbool.h>

#define BLANK_ELEM_BUTTON 0
#define BLANK_ELEM_GROUP 1

typedef struct {
  Blank_UiLayout layout;
  Blank_UiElement *ui_elems;
} Blank_UiElemGroup;

typedef struct {
  const char *text;
  bool disabled;
  OnClickFunc on_click_func;
} Blank_UiElemButton;

/* UI-Group functions */

void blank_ui_group(Blank_UiElemGroup *group, Blank_UiElement elem);

/* UI-Element creation functions */

Blank_UiElement blank_button(const char *text, bool disabled,
                             OnClickFunc on_click_func);

Blank_UiElement blank_group(Blank_UiElemGroup group);
