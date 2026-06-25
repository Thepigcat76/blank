#pragma once

#include "lilc/numbers.h"

typedef enum {
  BLANK_MOUSE_BUTTON_LEFT,
  BLANK_MOUSE_BUTTON_MIDDLE,
  BLANK_MOUSE_BUTTON_RIGHT,
} Blank_MouseButton;

typedef struct {
  i32 width;
  i32 height;
} Blank_Size;

struct blank_ui_element;

typedef void (*OnClickFunc)(struct blank_ui_element *elem, Blank_MouseButton mouse_button);

typedef u32 Blank_Color;

Blank_Color blank_color_make(u8 red, u8 green, u8 blue, u8 alpha);

u8 blank_color_red(Blank_Color color);

u8 blank_color_green(Blank_Color color);

u8 blank_color_blue(Blank_Color color);

u8 blank_color_alpha(Blank_Color color);

void blank_wait(int miliseconds);
