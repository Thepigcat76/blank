#pragma once

#include "lilc/numbers.h"
#include "lilc/alloc.h"
#include <stdbool.h>
#include "internal/blank_shared.h"
#include "internal/blank_init.h"

typedef struct blank_backend {
  void (*clear_screen_func)(struct blank_backend *, Blank_Color color);
  void (*render_rect_func)(struct blank_backend *, i32 x, i32 y, i32 width, i32 height, Blank_Color color);
  void (*window_init_func)(struct blank_backend *, const Blank_InitState *init_state);

  Allocator *ui_elem_allocator;
  Allocator *ui_state_allocator;

  // Custom context depending on the backend
  void *backend_context;
  
} Blank_Backend;
