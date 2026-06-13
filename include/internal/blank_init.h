#pragma once

#include "lilc/numbers.h"

typedef struct {
  const char *title;
  i32 width;
  i32 height;
  bool resizeable;
} Blank_InitState;

void blank_window_title(Blank_InitState *init_state, const char *title);

void blank_window_size(Blank_InitState *init_state, i32 width, i32 height);

void blank_window_resizeable(Blank_InitState *init_state);
